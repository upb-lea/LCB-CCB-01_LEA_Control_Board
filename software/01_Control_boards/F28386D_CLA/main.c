//=================================================================================================
/// @file			main.c
///
/// @brief		Diese Datei enth�lt das Hauptprogramm zur Demonstration des CLA-Moduls des
///						Mikrocontrollers TMS320F280049C. Es sind drei CLA-Tasks implementiert: der erste
///						Task ist Software-getriggert und dient zur Initialisierung des CLA-Moduls, da die
///						CPU (C28-Kern) nicht auf alle CLA-Register Zugriff hat. Der zweite Task wird durch
///						den ADC getriggert (Peripherie-getriggert) und setzt ein neues Tastverh�ltnis vom
///						ePWM1-Modul. Der dritte Task ist wieder Software-getriggert und demonstriert den
///						Austausch von Daten zwischen CPU und CLA. ePWM8 dient als Zeitgeber, um alle 10 ms
///						eine ADC-Messung zu triggern (Eingang ADCIN0). Das Ergebnis der Messung wird
///						proportional als Tastverh�ltnis an ePWM1A (Ausgang GPIO 0) mit einer Schaltfrequenz
///						von 10 kHz ausgegeben. Der CLA-Task 1 wird einmalig w�hrend der Initialisierung
///						(durch die CPU) ausgef�hrt. CLA-Task 2 wird alle 10 ms ausgef�hrt. CLA-Task 3 wird
///						einmalig ausgef�hrt, wenn die Variable "claStartTask3" auf 1 gesetzt wird. F�r
///						jeden Durchlauf von Task 3 muss "claStartTask3" auf 1 gesetzt werden.
///
///						HINWEIS: Das Programm startet nach erneutem Anlegen der Versorgungsspannung nicht,
///										 auch wenn es im FLASH gespeichert wurde. Lediglich wenn die CPU �ber den
///										 Debugger resettet und neu gestartet wird funktioniert es.
///
/// @version	V1.2
///
/// @date			08.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myCLA.h"
#include "myADC.h"
#include "myPWM.h"


// Grundlagen CLA:
// https://training.ti.com/cla-hands-workshop-part-1-introduction
// https://www.ti.com/lit/an/spracs0/spracs0.pdf
// https://software-dl.ti.com/C2000/docs/cla_software_dev_guide/_static/pdf/C2000_CLA_Software_Development_Guide.pdf
//
// Das CLA-Modul (Control Law Accelerator) ist eine von der CPU (C28-Kern) des
// Mikrocontrollers unabh�ngige, programmierbare 32-Bit Gleitzahl-CPU. Sie wird mit
// der selben Frequenz betrieben, wie die CPU (SYSCLK). Das CLA-Modul kann direkt auf
// verschiedene Module wie ADC, CMPSS und ePWM zugreifen. Regelaufgaben k�nnen so
// parallel zu der Haupt-CPU (C28-Kern) durchgef�hrt werden, wodurch deren Auslastung
// reduziert wird.
//
// Die Programmierung des CLA-Moduls erfolgt in C mit einem eigenen Compiler. Dieser wird
// automatisch aufgerufen, sobal eine Datei mit .cla endet. Die CPU muss nach dem Starten
// des Mikrocontrollers den Speicherplatz f�r das CLA-Modul reservieren*. Au�erdem muss
// muss die Initialisierung des CLA- und gff anderer Sub-Module (ADC, ePWM, etc.)
// ebenfalls die CPU durchf�hren.
//
// CLA und CPU (C28-Kern) k�nnen �ber zwei RAM-Speicherbereiche miteinander kommunizieren:
// CPUtoCLAMsgRAM: CPU kann den Speicherbereich lesen und beschreiben, CLA nur lesen
// CLAtoCPUMsgRAM: CLA kann den Speicherbereich lesen und beschreiben, CPU nur lesen
// Die Kommunikation erfolgt �ber Variablen, die im entsprechenden RAM-Bereich abgelegt werden
//
// Das CLA-Modul ist Task-gesteuert. Es gibt 8 Tasks mit Priorit�ten von 1 bis 8.
// Die h�chste Priorit�t hat Task Nr. 1, die geringste Priorit�t Task Nr. 8. Die Tasks
// k�nnen �ber Software oder durch Sub-Module (z.B. ADC) getriggert werden. Sobald
// ein Task getriggert wurde, wird er bis zum Ende durchgef�hrt. Er kann nicht
// durch einen anderen Task unterbrochen werden (keine nestet Tasks). Nach Abschluss
// eines Tasks kann ein CPU-Interrupt ausgel�st werden, z.B. f�r Post-Processing.
// Wird w�hrend der Ausf�hrung eines Tasks ein weiterer Task getriggert, wird der
// aktuelle Task zun�chst zuende gef�hrt und anschlie�end (innerhalb von 4 Takten)
// der getriggerte Task gestartet. Wurden mehrere Tasks getriggert, wird der Task
// mit der h�heren Priorit�t zuerst gestartet. Nachdem alle Tasks durchgef�hrt
// wurden, wechselt das CLA-Modul in eine Art Stand-by Zustand.
//
// Der Programmcode f�r das CLA-Modul wird in einer .cla-Datei gespeichert. Der
// CLA-Compiler unterst�tzt nicht die Standart C-Bibliothek. TI bietet als
// Ersatz eine Mathematik-Bibliothek an. Der Programmcode und die Konstanten
// des CLA-Moduls m�ssen nach dem Start des Mikrocontrollers vom Flash in den
// RAM kopiert werden (initCLA()). CLA und CPU teilen sich den selben RAM,
// daher ist die Zuweisung von Speicherbereichen durch die CPU notwendig**.
//
// Da sowohl die CPU als auch das CLA-Modul auf die selben Register zugreifen
// (z.B. ePWM-Register), muss sichergestellt werden, dass es zu keinen
// Datenkollisionen/�berschreibungen kommt.
//
// Der CLA-Code kann wie auch der CPU-Code gedebuggt werden. Nach dem Flashen
// muss jedoch die Debug-Probe manuell mit dem CLA-Modul verbunden werden. Dazu
// in der Debug-Ansicht mit Rechtsklick auf die CLA-Debug-Probe (befindet sich
// unterhalb der C28x-Debug-Probe) und "Connect target" w�hlen.
//
//
// *) Grundlagen Speicherbereiche:
// Der TMS320F2838x hat vier Speicher: Flash (Lesen und Schreiben, nicht-fl�chtig),
// RAM (Lesen und Schreiben, fl�chtig), ROM (Lesen, nicht fl�chtig) und Peripheral
// (Lesen und Schreiben, fl�chtig). Alle vier Speicher sind in verschiedene Bereiche
// eingeteilt: Im Peripheral-Speicher hat beispielsweise jede Peripherie (ADC, PWM,
// etc.) ihren eigenen Bereich. Im RAM gibt es dedizierte Bereiche f�r den CLA oder
// dem schnellen Zugriff durch die CPU. Im Flash gibt es Bereiche, in denen der
// Programmcode liegt sowie Bereiche, in denen das Hauptprogramm beliebige Daten
// abspeichern kann, die nach einer Spannungsunterbrechung wieder verf�gbar sind.
//
// Siehe S. 246 Datasheet TMS320F2838x:
//
// Flash: OTP (One-Time Programmable, einmalig beschreibbar, nicht l�schbar),
//        Bank 0 und Bank 1 (Programmcode), ECC (Error Correction Code).
//
// RAM:		Dedicated (nur CPU hat Zugriff), Local (nur CPU und CLA haben Zugriff),
//				Global (nur CPU und DMA haben Zugriff), CLA-Message (nur CPU und CLA
//				haben Zugriff).
//
// ROM:		Boot ROM (beinhaltet Code, welcher direkt nach einem Reset ausgef�hrt
//				wird und den Microcontroller so initialisiert, dass der Programmcode
//				geladen und ausgef�hrt wird), CLA Data ROM (Daten f�r die Berechnung
//				von z.B. Trigonometrischen Funktionen).
//
// Peripheral: Beinhaltet s�mtliche Register, auf welche die Pheripherie (z.B. ADC,
//						 ePWM, I2C, CLB, etc.) zugreift und welche zu dessen Konfiguration
//						 und Steuerung n�tig sind.
//
//
// **) Speicherplatz f�r CLA reservieren:
// Der Programmcode sowie die Daten (Variablen, etc.) f�r den CLA m�ssen im
// RAM gespeichert werden. Daf�r k�nnen nur die Bereiche LS0 bis LS7 verwendet
// werden (S. 182 TMS320F2838x Reference Manual). Jeder Speicherbereich LSx
// ist 2048 x 16 Bit gro� (S. 246 TMS320F2838x Datasheet)


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion initialisiert das CLA-Modul
void ClaInit(void);
// Interrupt-Service-Routine f�r den CLA-Task 1
__interrupt void ClaTask1Isr(void);
// Interrupt-Service-Routine f�r den CLA-Task 2
__interrupt void ClaTask2Isr(void);
// Interrupt-Service-Routine f�r den CLA-Task 3
__interrupt void ClaTask3Isr(void);


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Z�hlt die CPU-Interrupts des CLA-Task 1
uint16_t claInterrupt1Counter = 0;
// Z�hlt die CPU-Interrupts des CLA-Task 2
uint16_t claInterrupt2Counter = 0;
// Z�hlt die CPU-Interrupts des CLA-Task 3
uint16_t claInterrupt3Counter = 0;
// Startet den CLA-Task 3 per Software
uint16_t claStartTask3 = 0;
// Die folgenden Variablen m�ssen zus�tzlich noch in der h.Datei f�r das CLA-Modul
// deklariert werden. Es k�nnen dabei nur Datentypen aus der Bibliothek "stdint.h"
// verwendet werden, da das CLA-Modul einen eingeschr�nkten Befehlssatz hat. Die
// Variablen k�nnen hier nicht initialisiert werden, weil RAM-Initialisierung
// (MemCfgRegs.MSGxINIT.bit.INIT_x) den Wert der Variablen wieder �berschreibt.
// Variablen, die nur von CPU beschrieben und von CPU und CLA gelesen werden k�nnen.
#pragma DATA_SECTION(cpuToCla,"CpuToCla1MsgRAM");
unsigned int cpuToCla;
// Variablen, die nur von CLA beschrieben und von CPU und CLA gelesen werden k�nnen
#pragma DATA_SECTION(claToCpu,"Cla1ToCpuMsgRAM");
unsigned int claToCpu;


//=== Function: main ==============================================================================
///
/// @brief  Hauptprogramm
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void main(void)
{
		// Mikrocontroller initialisieren (Watchdog, Systemtakt, Speicher, Interrupts)
		DeviceInit(DEVICE_CLKSRC_EXTOSC_SE_25MHZ);
	  // ADC initialisieren
		AdcAInit(ADC_RESOLUTION_12_BIT,
						 ADC_SINGLE_ENDED_MODE);
	  // PWM initialisieren
	  PwmInitPwm1();
	  PwmInitPwm8();
	  // CLA initialisieren
	  ClaInit();

    // Zugriffsberechtigung von Variablen:
    // Hat keinen Effekt, da die Variable im RAM-Bereich "Cla1ToCpuMsgRAM"
    // liegt und somit nur von dem CLA-Kern ver�ndert werden kann
    claToCpu = 5;
    // Variable wird auf 2 gesetzt, da die Variable im RAM-Bereich
    // "CpuToCla1MsgRAM" liegt und nur von der CPU ver�ndert werden kann
    cpuToCla = 2;

    // Register-Schreibschutz aufheben
    EALLOW;


		// GPIO 5 (LED D1002 auf dem ControlBoard) als Ausgang
		// konfigurieren zur Visualisierung des ADC-Triggers
		EALLOW;
		// Konfigurationssperre f�r GPIO 5 aufheben
		GpioCtrlRegs.GPALOCK.bit.GPIO5 = 0;
		// GPIO 5 auf GPIO-Funktionalit�t setzen
		GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = (0 >> 2);
		GpioCtrlRegs.GPAMUX1.bit.GPIO5  = (0 & 0x03);
		// Pull-Up-Widerstand von GPIO 5 deaktivieren
		GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;
		// GPIO 5 auf High-Pegel setzen
		GpioDataRegs.GPASET.bit.GPIO5 = 1;
		// GPIO 5 als Ausgang setzen
		GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;


		// Dauerschleife Hauptprogramm
    while(1)
    {
    		// CLA-Task 3 �ber Software starten
    		if (claStartTask3 == 0)
    		{
    		    // CLA-Task 3 starten
    		    // MIFRC.INTx = 0: wird ignoriert
    		    // MIFRC.INTx = 1: CLA-Task starten
    				//EALLOW;
    		    Cla1Regs.MIFRC.bit.INT3 = 1;
    		    // Task nur einmal starten
    		    claStartTask3 = 0;

    		    DELAY_US(100000);
    		}
    }
}




//=== Function: ClaInit ===========================================================================
///
/// @brief	Funktion initialisiert das CLA-Modul
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void ClaInit(void)
{
		// CLA-Programmcode in den RAM kopieren. Die Namen der Variablen sind Platzhalter
		// f�r Konstanten des Linkers und d�rfen daher nicht ver�ndert werden.
    extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;
#ifdef _FLASH
		memcpy((uint32_t *)&Cla1funcsRunStart, (uint32_t *)&Cla1funcsLoadStart, (uint32_t)&Cla1funcsLoadSize);
#endif

		// Register-Schreibschutz aufheben
    EALLOW;

    // CPU-zu-CLA Message-Register Initialisierung:
    // Initialisierung starten (alle Speicherstellen auf 0 setzen)
    // 0: Keine Aktion
    // 1: Initialisierung starten
    MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;
    // Warten, bis die Initialisierung abgeschlossen ist
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 == 0);
    // CLA-zu-CPU  Message-Register Initialisierung starten
    MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;
    // Warten, bis die Initialisierung abgeschlossen ist
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU == 0);
    // CLA-DATENSPEICHER:
    // Zugriffsberechtigung RAM-LSx:
    // Zugriff durch CPU und CLA auf RAM LS0 freigeben
    // (siehe S. 182 Reference Manual TMS320F2838x)
    // 0: CPU hat Zugriff auf Speicher LSx
    // 1: CPU und CLA haben Zugriff auf Speicher LSx
    MemCfgRegs.LSxMSEL.bit.MSEL_LS0 = 1;
    // Funktion RAM-LSx als CLA-RAM:
    // LS0 als CLA-Datenspeicher verwenden (siehe S. 182 Reference Manual TMS320F2838x).
    // Welche Speicherbereiche als CLA-Datenspeicher zu deklarieren sind, ist dem Linker-
    // File zu entnehmen (Abschnitt "Cla1DataRam : >> RAMLS0 | RAMLS1").
    // 0: LSx als CLA-Datenspeicher
    // 1: LSx als CLA-Programmspeicher
    // Falls der Speicher nicht ausreichen sollte, erscheint die Compiler-Fehlermeldung
    // "Data will not fit into available memory". In diesem Fall muss mehr Speicher
    // f�r die Daten zugewiesen werden und das Linker-File angepasst werden. Daf�r den
    // im Linker-File den Abschnitt "Cla1DataRam : >> RAMLS0 | RAMLS1" suchen und um
    // die enstsprechenden Speicherstellen erweitern. Siehe dazu auch:
    // https://software-dl.ti.com/C2000/docs/C2000_Multicore_Development_User_Guide/faq.html
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS0 = 0;
    // Zugriff durch CPU und CLA auf RAM LS1 freigeben
    MemCfgRegs.LSxMSEL.bit.MSEL_LS1 = 1;
    // LS1 als CLA-Datenspeicher verwenden
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS1 = 0;
    // CLA-PROGRAMMSPEICHER:
    // Zugriff durch CPU und CLA auf RAM LS5 freigeben
    MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;
    // LS5 als CLA-Programmspeicher verwenden.
    // Welche Speicherbereiche als CLA-Programmspeicher zu deklarieren sind,
    // ist dem Linker- File zu entnehmen (Abschnitt "Cla1Prog")
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 1;

    // CLA-TASK 1 konfigurieren:
    // CLA-Task 1 dem CLA-Prozessor bekannt geben.
    // Jeder CLA-Task (wird in einer entsprechenden .cla-Datei definiert)
    // wird dem CLA-Kern �ber die Register MVECT1 bis MVECT7 bekannt gegeben
    Cla1Regs.MVECT1 = (uint16_t)&ClaTask1;
    // Software als Triggerquelle f�r CLA-Task 3 setzen.
    // (siehe S. 964 Reference Manual TMS320F2838x)
    // Trigger f�r Task 1 - 4: CLA1TASKSRCSEL1
    // Trigger f�r Task 5 - 8: CLA1TASKSRCSEL2
    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK1 = CLA_TASK_TRIGGER_SOFTWARE;
    // Task 1 freigegeben.
    // INTx = 0: Task gesperrt
    // INTx = 1: Task freigegeben
    Cla1Regs.MIER.bit.INT1 = 1;
    // Interrupt-Service-Routinen f�r den CLA-Task 1 Interrupt an die
    // entsprechende Stelle (CLA1_1_INT) der PIE-Vector Table speichern
    PieVectTable.CLA1_1_INT = &ClaTask1Isr;
    // INT11.1-Interrupt freischalten (Zeile 11, Spalte 1 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER11.bit.INTx1 = 1;

    // CLA-TASK 2 konfigurieren:
    // CLA-Task 2 dem CLA-Prozessor bekannt geben
    Cla1Regs.MVECT2 = (uint16_t)&ClaTask2;
    // ADC-A INT1 als Triggerquelle f�r CLA-Task 2 setzen
    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK2 = CLA_TASK_TRIGGER_ADCA_INT1;
    // Task 2 freigegeben
    Cla1Regs.MIER.bit.INT2 = 1;
    // Interrupt-Service-Routinen f�r den CLA-Task 2 Interrupt an die
    // entsprechende Stelle (CLA1_2_INT) der PIE-Vector Table speichern
    PieVectTable.CLA1_2_INT = &ClaTask2Isr;
    // INT11.2-Interrupt freischalten (Zeile 11, Spalte 2 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER11.bit.INTx2 = 1;

    // CLA-TASK 3 konfigurieren:
    // CLA-Task 3 dem CLA-Prozessor bekannt geben
    Cla1Regs.MVECT3 = (uint16_t)&ClaTask3;
    // Software als Triggerquelle f�r CLA-Task 3 setzen
    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK3 = CLA_TASK_TRIGGER_SOFTWARE;
    // Task 3 freigegeben
    Cla1Regs.MIER.bit.INT3 = 1;
    // Interrupt-Service-Routinen f�r den CLA-Task 3 Interrupt an die
    // entsprechende Stelle (CLA1_3_INT) der PIE-Vector Table speichern
    PieVectTable.CLA1_3_INT = &ClaTask3Isr;
    // INT11.3-Interrupt freischalten (Zeile 11, Spalte 3 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER11.bit.INTx3 = 1;

    // CPU-Interrupt 11 einschalten (Zeile 11 der Tabelle)
    IER |= M_INT11;

    // Initialisierungs-Task starten
    Cla1Regs.MIFRC.bit.INT1 = 1;

		// Register-Schreibschutz setzen
		EDIS;
}


//=== Function: ClaTask1Isr =======================================================================
///
/// @brief	ISR wird aufgerufen, nachdem der CLA-Task 1 beendet wurde oder das CLA-Modul manuell
///					den Interrupt ausgel�st hat (Cla1OnlyRegs.SOFTINTFRC.bit.TASK1 = 1).
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void ClaTask1Isr(void)
{
		// Interrupt z�hlen
		claInterrupt1Counter++;

		// Interrupt-Flag der Gruppe 11 l�schen (da geh�rt der CLA1_1_INT-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK11 = 1;
}


//=== Function: ClaTask2Isr =======================================================================
///
/// @brief	ISR wird aufgerufen, nachdem der CLA-Task 2 beendet wurde oder das CLA-Modul manuell
///					den Interrupt ausgel�st hat (Cla1OnlyRegs.SOFTINTFRC.bit.TASK2 = 1).
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void ClaTask2Isr(void)
{
		// Interrupt z�hlen
		claInterrupt2Counter++;

		// Interrupt-Flag der Gruppe 11 l�schen (da geh�rt der CLA1_1_INT-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK11 = 1;
}


//=== Function: ClaTask3Isr =======================================================================
///
/// @brief	ISR wird aufgerufen, nachdem der CLA-Task 3 beendet wurde oder das CLA-Modul manuell
///					den Interrupt ausgel�st hat (Cla1OnlyRegs.SOFTINTFRC.bit.TASK3 = 1).
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void ClaTask3Isr(void)
{
		// Interrupt z�hlen
		claInterrupt3Counter++;

		GpioDataRegs.GPATOGGLE.bit.GPIO5 = 1;

		// Interrupt-Flag der Gruppe 11 l�schen (da geh�rt der CLA1_1_INT-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK11 = 1;
}
