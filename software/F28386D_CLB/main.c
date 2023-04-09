//=================================================================================================
/// @file			main.c
///
/// @brief		Diese Datei enth�lt das Hauptprogramm zur Demonstration des CLB-Moduls f�r den
///						Mikrocontroller TMS320F2838x. Die Konfiguration der CLB-Logik wird �ber die
///						Datei "emty.syscfg" vorgenommen, siehe dazu unten den Abschnitt "Grundlagen CLB".
///						Es wird die CLB1-Instanz verwendet. Als Logik wird ein UND-Gatter mit zwei Eing�ngen
///						und einem Ausgnag implementiert. Die beiden Eing�nge sind GPIO 0 und 1, der Ausgang
///						ist GPIO 2. Hinweis: Dieses Projekt ist Driverlib-basiert, weil die .syscfg-Datei
///						Header- und Source-Dateien erzeugt, welche auf die Driverlib zugreift.
///
/// @version	V1.1
///
/// @date			08.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"
#include "clb_config.h"


// Grundlagen CLB:
// https://www.ti.com/lit/ug/spruir8a/spruir8a.pdf
// https://www.ti.com/lit/an/spracl3/spracl3.pdf
//
//
// Das CLB-Modul (Configurable Logic Block = FPGA) besteht aus
// einem oder mehreren identischen (Teil-)Instanzen, s.g. Tiles.
//
// Der TMS320F28386/8 besitzt 8 CLB-Instanzen.
//
// Jedes CLB-Modul besteht aus verschieden Sub-Modulen:
// - LUT (Look Up Table)        : Wahrheitstabelle
// - FSM (Fenite State Machine) : Zustandsmaschine
// - Counter                    : Z�hler
// - Output LUT                 : Ausgangs-Wahrheitstabelle (�ber diese werden die CLB-
//																                           Ausgangssignale aus der CLB-
//																													 Instanz herausgef�hrt)
// - HLC (High Level Controller): Event- und Interrupt-Logikblock
// - AOC (Asynchronous Output
//        Conditioning Block)   : Verarbeitung und Transport (zu SYSCLK) asynchroner Signale
//
// Jede CLB-Instanz besitzt 8 Eing�nge und 8 Ausg�nge. Die Ausg�nge sind beim TMS320F2838x
// 4-fach repliziert, sodass insgesamt 32 Ausg�nge zur Verf�gung stehen (siehe S. 1190
// Reference Manual TMS320F2838x). Somit f�hren die Ausg�nge OUT0, OUT8, OUT16 und OUT24
// dasselbe Signal.
//
// Auf die Eing�nge k�nnen diverse Signale des Mikrocontrollers gelegt werden.
// Es gibt "Global Inputs" (Eing�nge, die allen CLB-Instanzen zur Verf�gung stehen)
// und "Local Inputs" (Eing�nge, die f�r jede CLB-Instanz spezifisch sind).
//
// Die Ausg�nge enthalten die verarbeiteten Informationen der CLB-Instanz und k�nnen
// �ber einen Demultiplexer auf diverse Peripherisignale gelegt werden. So kann z.B.
// ein PWM-Signal entweder vom ePWM-Modul stammen oder von einer CLB-Instanz. Welches
// von beiden Signalen an den GPIOs ausgegben wird, legt ein Multiplexer fest.
//
// Die Logik einer CLB-Instanz kann mithilfe eines CLB-Tools erstellt werden.
// Dieses Programm erzeugt eine .h- und eine .c-Datei, mit der die CLB-Register
// beschrieben werden um die gew�nschten Logik-Funktionen abzubilden. Die Eingangs-
// und Ausgangsverbindungen der CLB-Instanzen werden nicht durch das Programm
// erstellt, sondern m�ssen manuell programmiert werden.
//
// Das CLB-Tool nutzt die Oberfl�che des Programms "Sys-Config". Ggf. muss
// dieses noch installiert werden. Wichtig: Sys-Config muss zwingend unter
// "C:/ti/ccs<ccs-version>/ccs/utils/sysconfig_<syscfg-version>" installiert sein!
//
// Um das CLB-Tool zu �ffnen, muss eine .syscfg-Datei vorliegen. Am einfachsten
// wird dazu �ber den Resource Explorer das Driverlib-basierte Projekt "clb_empty"
// importiert. In diesem Projekt befindet sich die Datei "empty.syscfg".
//
// Die CLB-Submodule (LUT, Counter, etc.) entsprechend der Anforderungen konfigurieren
// -> .syscfg-File speichern -> Dateien "clb_config.h" und "clb_config.c" werden im
// Projektpfad CPU1_RAM/syscfg bzw. CPU1_FLASH/syscfg nach dem Kompilieren gespeichert
// -> "clb_config.h" im Hauptprogramm inkludieren.
//
// Im Hauptprogramm zun�chst den Takt f�r das verwendete CLB-Modul (1 ... 8) einschalten
// (PCLKCR17.bit.CLBx), die Initalisierungsfunktion aufrufen (Default-Name "initTILE1()",
// der tats�chliche Name kann der Datei "clb_config.h" entnommen werden), die Logik
// freischalten (ClbxLogicCtrlRegs.CLB_LOAD_EN.bit.GLOBAL_EN) und die Ein- und Ausg�nge
// der CLB-Instanz konfigurieren.
//
// Die Eingangssignale werden der CLB-Instanz �ber die CLB X-Bar zugef�hrt. Alternativ
// k�nnen auch Ausgangssignale von anderen CLB-Intanzen als Eingangssignal gew�hlt werden.
//
// Die Ausgangssignale einer CLB-Instanz werden �ber die Output Look-Up-Tables
// herausgef�hrt und k�nnen �ber die verschiedenen X-Bars (ePWM, Output) oder
// dem "Peripheral Signal Multiplexer" auf interne Signale des Mikrocontrollers
// gelegt werden.


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Aus hw_memmap.h
#define CLB1_BASE                 0x00003000U
#define CLB2_BASE                 0x00003200U
#define CLB3_BASE                 0x00003400U
#define CLB4_BASE                 0x00003600U
#define CLB5_BASE                 0x00003800U
#define CLB6_BASE                 0x00003A00U
#define CLB7_BASE                 0x00003C00U
#define CLB8_BASE                 0x00003E00U


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


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

		// Register-Schreibschutz aufheben
		EALLOW;
    // Takt f�r das CLB1-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 283 Reference Manual TMS320F2838x)
		CpuSysRegs.PCLKCR17.bit.CLB1 = 1;
    __asm(" RPT #4 || NOP");
    // CLB-Logik und -Register synchron zum Systemtakt betreiben
    // 0: CLB-Modul synchron zu SYSCLK
    // 1: CLB-Modul asynchron zu SYSCLK
    ClkCfgRegs.CLBCLKCTL.bit.CLKMODECLB1 = 0;
    /*
    // Die CLB-Clock darf max. 150 MHz betragen und �ber 100 MHz muss der PIPELINE-Modus
    // aktiviert werden (siehe S. 1176 Reference Manual TMS320F2838x). Da in der Funktion
    // "DeviceInit" der Vorteiler PERCLKDIVSEL.EPWMCLKDIV auf 2 gesetzt und SYSCLK 200 MHz
    // betr�gt ist sichergestellt, dass die Anforderungen erf�llt sind.
    // Vorteiler auf 2 setzen
    // 0: Teiler = 1
    // 1: Teiler = 2
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    // Taktteiler f�r die CLB-Register
    ClkCfgRegs.CLBCLKCTL.bit.CLBCLKDIV = 0;
    // Taktteiler f�r die CLB-Logik
    // 0: Teiler = 1
    // 1: Teiler = 2
    // ...
    // 8: Teiler = 8
    ClkCfgRegs.CLBCLKCTL.bit.TILECLKDIV = 0;
    */
    // CLB-Block initialisieren
    initTILE1(CLB1_BASE);
    // Nach dem Funktionsaufruf muss der
    // Schreibschutz erneut aufgehoben werden
    EALLOW;
    // CLB-Modul einschalten
    Clb1LogicCtrlRegs.CLB_LOAD_EN.bit.GLOBAL_EN = 1;
    // Alle CLB_LOGIC_CONFIG_REGS- und einige der
    // CLB_LOGIC_CONTROL_REGS-Register k�nnen nur
    // mit deaktiviertem Schreibschutz bearbeitet
    // werden (Clb1LogicCtrlRegs.CLB_LOCK.bit.LOCK = 0).
    // Das Bit kann nur gesetzt werden und das nur einmalig.
    // Gel�scht wird es durch einen Reset (siehe S. 1279
    // Reference Manual TMS320F2838x)


    //*****************************************************
    // CLB-Eing�nge konfigurieren:
    //*****************************************************
    // Eingang 0 / BOUNDARY IN0 (erstes Eingangssignal) konfigurieren:
    // Signal AUXSIG0 (CLB X-Bar) als Eingangssignal des globalen Eingangs
    // setzen (siehe Figure 9-5 und Table 9-2 S. 1178 ff. Reference Manual
    // TMS320F2838x)
    // GLBL_MUX_SEL_IN_0: Eingang 0
    // ...
    // GLBL_MUX_SEL_IN_7: Eingang 7
    Clb1LogicCtrlRegs.CLB_GLBL_MUX_SEL_1.bit.GLBL_MUX_SEL_IN_0 = 64;
    // Globalen Eingang 0 verwenden
    // 0: Globalen Eingang
    // x: Lokalen Eingang
    // LCL_MUX_SEL_IN_0: Eingang 0
    // ...
    // LCL_MUX_SEL_IN_7: Eingang 7
    Clb1LogicCtrlRegs.CLB_LCL_MUX_SEL_1.bit.LCL_MUX_SEL_IN_0 = 0;
    // Signalsynchronisation einschalten.
    // Muss f�r alle Signale eingeschaltet werden, die nach den Tabellen
    // "Table 9-2 Global Signals and Mux Selection" und "Table 9-3 Local
    // Signals and Mux Selection" mit "ASYNC" markiert sind (siehe S. 1179
    // ff. bzw. S. 1185 ff. Reference Manual TMS320F2838x)
    // 0: Synchronisation ausgeschaltet
    // 1: Synchronisation eingeschaltet
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.SYNC0 = 1;
    // Keine Signalfilterung verwenden
    // 0: Keine Filterung
    // 1: Steigende Flanken durchlassen
    // 2: Fallende Flanken durchlassen
    // 4: Beide Flanken durchlassen
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.FIN0 = 0;
    // Das zuvor konfigurierte (externe) Signal verwenden
    // 0: BOUNDARY INx ist externes Signal
    // 1: BOUNDARY INx ist GPREGx
    Clb1LogicCtrlRegs.CLB_IN_MUX_SEL_0.bit.SEL_GP_IN_0 = 0;
    // Eingang 1 / BOUNDARY IN1 (zweites Eingangssignal) konfigurieren:
    // Signal AUXSIG1 (CLB X-Bar) als globalen Eingang setzen
    Clb1LogicCtrlRegs.CLB_GLBL_MUX_SEL_1.bit.GLBL_MUX_SEL_IN_1 = 65;
    // Globalen Eingang 1 verwenden
    Clb1LogicCtrlRegs.CLB_LCL_MUX_SEL_1.bit.LCL_MUX_SEL_IN_1 = 0;
    // Signalsynchronisation einschalten
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.SYNC1 = 1;
    // Keine Signalfilterung verwenden
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.FIN1 = 0;
    // Das zuvor konfigurierte (externe) Signal verwenden
    Clb1LogicCtrlRegs.CLB_IN_MUX_SEL_0.bit.SEL_GP_IN_1 = 0;


		//*****************************************************
		// Eingangs-GPIO konfigurieren:
		//*****************************************************
		// GPIO 0 und GPIO 1 sind die Eing�nge der CLB-Instanz.
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO0 = 0;
    GpioCtrlRegs.GPALOCK.bit.GPIO1 = 0;
    // Auf GPIO-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = (0x00 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO0  = (0x00 & 0x03);
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = (0x00 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO1  = (0x00 & 0x03);
    // Pull-Up-Widerstand aktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0;
    // Samplingperiode auf 2*SYSCLK setzen
    GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = 1;
    // GPIOs mit jedem Sampleimpuls einlesen
    GpioCtrlRegs.GPAQSEL1.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0;
    // Als Eingang setzen
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 0;


    //*****************************************************
    // Input X-Bar konfigurieren:
    //*****************************************************
    // Die Input X-Bar hat 16 Ausg�nge, auf die jeweils einer der
    // GPIOs des Mikrocontrollers gelegt werden kann. Die Signale
    // der GPIOs k�nnen nicht invertiert werden, wie beispielsweise
    // bei der CLB oder ePWM X-Bar.
    // GPIO 0 auf Input1 legen
    // 0 = GPIO 0
    // ...
    // n = GPIO n
    InputXbarRegs.INPUT1SELECT = 0;
    // GPIO 1 auf Input2 legen
    InputXbarRegs.INPUT2SELECT = 1;


    //*****************************************************
    // CLB X-Bar konfigurieren:
    //*****************************************************
    // Die CLB X-Bar hat 8 Ausg�nge (AUXSIG0 bis AUXSIG7), die jeweils
    // bis zu 32 Signale verodern. Jeder der 32 Multiplexer (MUX0 ...
    // MUX31) kann aus bis zu 4 Signalen w�hlen. Je nach gew�nschtem
    // Signal muss der entsprechende Multiplexer ausgew�hlt werden. An-
    // schlie�end muss das Ausgangssignal des Multiplexers noch durchge-
    // schleift werden. Optional kann das aus den 32 Signalquellen vero-
    // derte Gesamtsignal noch invertiert werden (siehe S. 2146  ff.
    // Reference Manual TMS320F2838x). Die erste Spalte der Tabelle 17-4
    // gibt an, welcher Multiplexer (0 ... 15) f�r das gew�nschte Signal
    // verwendet werden muss. Dabei ist darauf zu achten, dass jedes CLB-Signal
    // (AUXSIGn) einen entsprechenden Multiplexer hat. Die oberste Zeile
    // der Tabelle gibt den Registerwert f�r den entsprechenden Multiplexer an.
    // INPUT1 der Input X-Bar auf den AUXSIG0-Ausgang legen
    CLBXbarRegs.AUXSIG0MUX0TO15CFG.bit.MUX1 = 1;
    // MUX1 (INPUT1-Signal) durchschalten
    // 0: Signal gesperrt
    // 1: Signal durchgeschaltet
    CLBXbarRegs.AUXSIG0MUXENABLE.bit.MUX1 = 1;
    // Signal nicht invertieren
    // 0: Signal nicht invertieren
    // 1: Signal invertieren
    CLBXbarRegs.AUXSIGOUTINV.bit.OUT0 = 0;
    // INPUT2 der Input X-Bar auf den AUXSIG1-Ausgang legen
    CLBXbarRegs.AUXSIG1MUX0TO15CFG.bit.MUX3 = 1;
    // MUX3 (INPUT2-Signal) durchschalten
    CLBXbarRegs.AUXSIG1MUXENABLE.bit.MUX3 = 1;
    // Signal nicht invertieren
    CLBXbarRegs.AUXSIGOUTINV.bit.OUT1 = 0;


    //*****************************************************
    // CLB-Ausg�nge konfigurieren:
    //*****************************************************
    // Falls Peripherie-Signale �bersteuert werden solllen, muss das entsprechende
    // Bit im Multiplexer gesetzt werden. Je nach dem, welches Peripherie-Signal
    // beeinflusst werden soll, muss die entsprechende Output Look-Up-Table ausge-
    // w�hlt und der Multiplexer gesetzt werden (siehe S. 1191 ff. Reference Manual
    // TMS320F2838x).
    // Ausgang 0 (Output LUT 0) auf das ePWM1A-Signal legen
    //Clb1LogicCtrlRegs.CLB_OUT_EN = 1;


    //*****************************************************
    // Output X-Bar konfigurieren:
    //*****************************************************
    // Die Output X-Bar hat 8 Ausg�nge (OUTPUT1 bis OUTPUT8), die jeweils
    // bis zu 32 Signale f�hren k�nnen. Jeder der 32 Multiplexer (MUX0
    // ... MUX31) kann aus bis zu 4 Signalen w�hlen. Je nach gew�nschtem
    // Signal muss der entsprechende Multiplexer ausgew�hlt werden. An-
    // schlie�end muss das Ausgangssignal des Multiplexers noch durchge-
    // schleift werden. Optional kann das aus den 32 Signalquellen vero-
    // derte Gesamtsignal noch �ber ein latch gef�hrt sowie invertiert
    // werden. Siehe S. 2148  ff. Reference Manual TMS320F2838x. Die erste
    // Spalte der Tabelle 17-5 gibt an, welcher Multiplexer f�r das gew�nschte
    // Signal verwendet werden muss.
    // Nach Tabelle 15-7 (S. 1645 Reference Manual TMS320F2838x) kann auf GPIO 2
    // nur der Ausgang 1 der Output X-Bar gelegt werden. Daher wird dieser im
    // Folgenden konfiguriert.
    // CLB1_OUT4 auf X-Bar Output 1 legen
    OutputXbarRegs.OUTPUT1MUX0TO15CFG.bit.MUX1 = 2;
    // MUX1 durchschalten
    OutputXbarRegs.OUTPUT1MUXENABLE.bit.MUX1 = 1;
    // Latch umgehen
    OutputXbarRegs.OUTPUTLATCHENABLE.bit.OUTPUT1 = 0;
    // Signal nicht invertieren
		OutputXbarRegs.OUTPUTINV.bit.OUTPUT1 = 0;


		//*****************************************************
		// Ausgangs-GPIO konfigurieren:
		//*****************************************************
    // GPIO 2 ist der Ausgang der CLB-Instanz.
    // Konfigurationssperre aufheben
		// (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPALOCK.bit.GPIO2 = 0;
    // Auf Output X-Bar Funktionalit�t setzen (OUTPUTXBAR3)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = (0x05 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO2  = (0x05 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;
    // Als Ausgang setzen
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;


		// Dauerschleife Hauptprogramm
    while(1)
    {

    }
}

