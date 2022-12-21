//=================================================================================================
/// @file       myPWM.c
///
/// @brief      Datei enth�lt Variablen und Funktionen um die Funktion der ePWM-Module zu
///							demonstrieren. Die ePWM-Module 1 bis 3 werden so initialisiert, dass damit
///							ein 3-phasiger Wechselrichter angesteuert werden kann. Das ePWM8-Modul wird
///							so initialisiert, dass alle 100 ms ein Interrupt ausgel�st wird und so als
///							Zeitgeber f�r periodisch zu erledigende Aufgaben genutzt werden kann.
///
/// @version    V1.1
///
/// @date       06.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myPWM.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: PwmInitPwm123 =====================================================================
///
/// @brief  Funktion initialisiert das ePWM1, ePWM2 und ePWM3-Modul um drei Halbbr�cken
///					eines 3-phasigen Wechselrichter anzusteuern. Das ePWM1-Modul ist dabei der
///					Master und synchronisiert die anderen zwei Halbbr�cken mit sich. Es Wird der
///					Auf-/Abz�hlmodus gesetzt. So ist sichergestellt, dass nie alle drei Halbbr�cken
///					zum selben Zeitpunkt umschalten. Dies ist vorteilhaft f�r die Kommutierung
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void PwmInitPwm123(void)
{
    // Register-Schreibschutz aufheben
    EALLOW;
    // Synchronisierungstakt w�hrend der Konfiguration ausschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    // PWM1-Modul:
    // Takt f�r das PWM1-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    __asm(" RPT #4 || NOP");
    // Takt-Teiler des PWM-Moduls setzen
    // TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)
    // EPWMCLK = SYSCLK / 2 = 100 MHz
    // (siehe "DeviceInit()" und S. 165 Reference Manual TMS320F2838x)
    EPwm1Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_1;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_1;
    // TBCTR nicht mit Wert aus dem Phasenregister laden
    EPwm1Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_DISABLE;
    // Kein Phasenversatz
    EPwm1Regs.TBPHS.bit.TBPHS = 0;
    // Synchronisations-Impuls (SYNCOUT) erzeugen, wenn Timer = 0
    // (siehe S. 2865 Reference Manual TMS320F2838x)
    EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;
    // Kontinuierlicher Synchronisationsbetrieb
    EPwm1Regs.TBCTL2.bit.OSHTSYNCMODE = 0;
    // Betriebsart: hoch-runter z�hlen
    EPwm1Regs.TBCTL.bit.CTRMODE = PWM_TB_COUNT_UPDOWN;
    // Wert, welcher in TBPRD geschrieben wird, sofort �bernehmen
    EPwm1Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periode f�r 16 kHz Schaltfrequenz setzen
    // bei 100 MHz EPWMCLK und Auf-Abz�hlmodus
    EPwm1Regs.TBPRD = PWM_PERIOD;
    // Compare-Register erst beschreiben, wenn der Z�hler den Wert 0 erreicht
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = PWM_CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = PWM_CC_SHDW_CTR_ZERO;
    // Tastverh�ltnis auf 0 setzen. Dieser Wert wird kontinuierlich mit
    // TBCTR (Timer-Z�hlwert) verglichen. Sind beide Werte gleich, wird
    // abh�nging von AQCTLA der PWM1A-Pin gesetzt
    EPwm1Regs.CMPA.bit.CMPA = 0;
    // PWM1A-Pin auf high ziehen, wenn TBCTR (Timer-Z�hlwert) beim hochz�hlen den Wert CMPA erreicht
    EPwm1Regs.AQCTLA.bit.CAU = PWM_AQ_SET;
    // PWM1A-Pin auf low ziehen, wenn TBCTR (Timer-Z�hlwert) beim runterz�hlen den Wert CMPA erreicht
    EPwm1Regs.AQCTLA.bit.CAD = PWM_AQ_CLEAR;
    // Totzeiten (siehe S. 2898 Reference Manual TMS320F2838x):
    // An GPIO0 wird das PWM1A-Signal mit einer um DBRED verz�gerten steigenden
    // Fanke ausgegeben. An GPIO1 wird das invertierte PWM1A-Signal mit einer
    // um DBFED verz�gerten fallenden Flanke ausgegeben. Somit h�ngt das Signal
    // an GPIO1 direkt von PWM1A ab und der Kanal B (CMPB und AQCTLB) muss nicht
    // initialisiert werden.
    // Totzeit-Z�hler l�uft mit der selben Frequenz wie der PWM-Z�hler
    EPwm1Regs.DBCTL.bit.HALFCYCLE = 0;
    // Das Eingangssignal f�r beide Totzeit-Z�hler ist PWM1A
    // (steigende und fallende Flanke)
    EPwm1Regs.DBCTL.bit.IN_MODE = PWM_DB_IN_A_ALL;
    // PWM1A-Signal nach Totzeitverz�gerung steigende Flanke nicht invertieren,
    // PWM1A-Signal nach Totzeitverz�gerung fallende Flanke invertieren
    EPwm1Regs.DBCTL.bit.POLSEL = PWM_DB_POL_B_INV;
    // PWM1A-Ausgangssignal: PWM1A-Signal mit verz�gerter steigender Flanke
    // PWM1B-Ausgangssignal: PWM1A-Signal mit verz�gerter fallender Flanke, invertiert
    EPwm1Regs.DBCTL.bit.OUT_MODE = PWM_DB_NONE_BYPASSED;
    // Totzeit f�r steigende Flanke
    EPwm1Regs.DBRED.bit.DBRED = PWM_DEAD_BAND;
    // Totzeit f�r fallende Flanke
    EPwm1Regs.DBFED.bit.DBFED = PWM_DEAD_BAND;
    // Z�hler auf 0 setzen
    EPwm1Regs.TBCTR = 0;
    // PWM1-Interrupt einschalten
    EPwm1Regs.ETSEL.bit.INTEN = 1;
    // PWM1-Interrupt ausl�sen, wenn der Z�hler den Wert 0 erreicht
    EPwm1Regs.ETSEL.bit.INTSEL = PWM_ET_CTR_ZERO;
    // PWM1-Interrupt beim ersten Auftreten des Events ausl�sen
    EPwm1Regs.ETPS.bit.INTPRD = PWM_ET_1ST;
		// CPU2 steuert das ePWM1-Modul
		// 0: CPU1 steuert das Modul
		// 1: CPU2 steuert das Modul
		// Die Umschaltung der CPU ist Glitch-behaftet. Daher sollte
		// der Takt zu der entsprechenden Peripherie erst nach der
		// Zuweisung der steuernden CPU durchgeschaltet werden
		// (siehe S. 428 ff. Reference Manual TMS320F2838x)
    //DevCfgRegs.CPUSEL0.bit.EPWM1 = 1;
    // CPU-Interrupts w�hrend der Konfiguration global sperren
    DINT;
    // Interrupt-Service-Routinen f�r den ePWM1-Interrupt an die
    // entsprechende Stelle (ePWM1_INT) der PIE-Vector Table speichern
    PieVectTable.EPWM1_INT = &Pwm1ISR;
    // INT3.1-Interrupt freischalten (Zeile 3, Spalte 1 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    // CPU-Interrupt 3 einschalten (Zeile 3 der Tabelle)
    IER |= M_INT3;
    // CPU-Interrupts nach Konfiguration global wieder freigeben
    EINT;


    // PWM2-Modul:
    // Takt f�r das PWM2-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    __asm(" RPT #4 || NOP");
    // Takt-Teiler des PWM-Moduls setzen
    // TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)
    // EPWMCLK = SYSCLK / 2 = 100 MHz
    // (siehe "DeviceInit()" und S. 165 Reference Manual TMS320F2838x)
    EPwm2Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_1;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_1;
    // Synchronisation mit Synchronisationsausgang von ePWM1
    // (siehe S. 2865 Reference Manual TMS320F2838x)
    EPwm2Regs.EPWMSYNCINSEL.bit.SEL = PWM_TB_SYNCIN_EPWM1_SYNCOUT;
    // TBCTR mit Wert aus dem Phasenregister laden,
    // wenn ein Synchronisationsimpuls auftritt
    EPwm2Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_ENABLE;
    // Wert, welcher bei einem Synchronisationsimpuls in das
    // Z�hlerregister (TBCTR) geladen wird. Falls der PWM-Takt
    // gleich der Z�hler-Takt ist (CLKDIV = 1, HSPCLKDIV = 1 ->
    // EPWMCLK = TBCLK), wird der Wert aus dem Phasen-Register (TBPHS)
    // 2 Taktzyklen nach dem Synchronisationsimpuls in das Z�hler-
    // Register geladen. Falls der Z�hlertakt kleiner als der PWM-Takt
    // ist (CLKDIV > 1 und/oder HSPCLKDIV > 1 -> EPWMCLK > TBCLK),
    // betr�gt die Verz�gerung 1 Taktzyklus. Im aktuellen Datenblatt
    // (Reference Manual TMS320F2838x) steht dieses Verhalten nicht
    // beschrieben, es ist jedoch z.B. auf S. 1783 im Reference Manual
    // TMS320F280049C dokumentiert. Zudem konnte diese Verhalten
    // nachgewiesen werden. Im Up-Modus kann diese Verz�gerung durch
    // TBPHS = 2 bzw. TBPHS = 1 kompensiert werden. Im Up-Down-Modus
    // funktioniert das nicht, weil das Slave-PWM-Modul ggf. runter
    // z�hlt w�hrend der Synchronisationsimpuls kommt. In diesem Fall
    // w�rde der Z�hler z.B. von 0 auf 2 bzw. 1 zur�ckspringen und von
    // dort aus wieder runtergez�hlt werden. F�r diesen Betriebsfall
    // kann das Bit TBCTL.bit.PHSDIR verwendet werden. Dieses Bit ist
    // nur wirksam, falls der Up-Dowm-Modus akiv ist. Es legt die Z�hl-
    // richtung des PWM-Moduls fest, nachdem ein Synchronisationsevent
    // aufgetreten ist und ein Wert aus dem Register TBPHS geladen wurde.
    // Alternative: Zwei Compare-Register verwenden (CMPA und CMPB).
    // Beim Hochz�hlen wird dann beispielsweise der Ausgang auf 1
    // umgeschaltet, wenn TBCTR = CMPA und beim Runterz�hlen auf 0, wenn
    // TBCTR = CMPB. Die CMPA-Register der Slave-PWM-Module werden um
    // den Wert der Verz�gerungszeit (2 oder 1) reduziert und die CMPB-
    // Register um den Wert (2 oder 1) erh�ht
    EPwm2Regs.TBPHS.bit.TBPHS = 0;
    // Wert, welcher in TBPRD geschrieben wird, sofort �bernehmen
    EPwm2Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periode f�r 16 kHz Schaltfrequenz setzen
    // bei 100 MHz EPWMCLK und Auf-Abz�hlmodus
    EPwm2Regs.TBPRD = PWM_PERIOD;
    // Betriebsart: hoch-runter z�hlen
    EPwm2Regs.TBCTL.bit.CTRMODE = PWM_TB_COUNT_UPDOWN;
    // Compare-Register erst beschreiben, wenn der Z�hler den Wert 0 erreicht
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = PWM_CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = PWM_CC_SHDW_CTR_ZERO;
    // Tastverh�ltnis auf 0 setzen
    // Dieser Wert wird kontinuierlich mit TBCTR (Timer-Z�hlwert) verglichen.
    // Sind beide Werte gleich, wird abh�nging von AQCTLA der PWM2A-Pin gesetzt
    EPwm2Regs.CMPA.bit.CMPA = 0;
    // PWM2A-Pin auf high ziehen, wenn TBCTR (Timer-Z�hlwert) beim hochz�hlen den Wert CMPA erreicht
    EPwm2Regs.AQCTLA.bit.CAU = PWM_AQ_SET;
    // PWM2A-Pin auf low ziehen, wenn TBCTR (Timer-Z�hlwert) beim runterz�hlen den Wert CMPB erreicht
    EPwm2Regs.AQCTLA.bit.CBD = PWM_AQ_CLEAR;
    // Totzeiten (siehe S. 2898 Reference Manual TMS320F2838x):
    // An GPIO2 wird das PWM2A-Signal mit einer um DBRED verz�gerten steigenden
    // Fanke ausgegeben. An GPIO3 wird das invertierte PWM2A-Signal mit einer
    // um DBFED verz�gerten fallenden Flanke ausgegeben. Somit h�ngt das Signal
    // an GPIO3 direkt von PWM2A ab und der Kanal B (CMPB und AQCTLB) muss nicht
    // initialisiert werden.
    // Totzeit-Z�hler l�uft mit der selben Frequenz wie der PWM-Z�hler
    EPwm2Regs.DBCTL.bit.HALFCYCLE = 0;
    // Das Eingangssignal f�r beide Totzeit-Z�hler ist PWM2A
    // (steigende und fallende Flanke)
    EPwm2Regs.DBCTL.bit.IN_MODE = PWM_DB_IN_A_ALL;
    // PWM2A-Signal nach Totzeitverz�gerung steigende Flanke nicht invertieren,
    // PWM2A-Signal nach Totzeitverz�gerung fallende Flanke invertieren
    EPwm2Regs.DBCTL.bit.POLSEL = PWM_DB_POL_B_INV;
    // PWM2A-Ausgangssignal: PWM2A-Signal mit verz�gerter steigender Flanke
    // PWM2B-Ausgangssignal: PWM2A-Signal mit verz�gerter fallender Flanke, invertiert
    EPwm2Regs.DBCTL.bit.OUT_MODE = PWM_DB_NONE_BYPASSED;
    // Totzeit f�r steigende Flanke
    EPwm2Regs.DBRED.bit.DBRED = PWM_DEAD_BAND;
    // Totzeit f�r fallende Flanke
    EPwm2Regs.DBFED.bit.DBFED = PWM_DEAD_BAND;
    // Z�hler auf 0 setzen
    EPwm2Regs.TBCTR = 0;


    // PWM3-Modul:
    // Takt f�r das PWM3-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR2.bit.EPWM3 = 1;
    __asm(" RPT #4 || NOP");
    // Takt-Teiler des PWM-Moduls setzen
    // TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)
    // EPWMCLK = SYSCLK / 2 = 100 MHz
    // (siehe "DeviceInit()" und S. 165 Reference Manual TMS320F2838x)
    EPwm3Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_1;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_1;
    // Synchronisation mit Synchronisationsausgang von ePWM1
    // (siehe S. 2865 Reference Manual TMS320F2838x)
    EPwm3Regs.EPWMSYNCINSEL.bit.SEL = PWM_TB_SYNCIN_EPWM1_SYNCOUT;
    // TBCTR mit Wert aus dem Phasenregister laden,
    // wenn ein Synchronisationsimpuls auftritt
    EPwm3Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_ENABLE;
    // Wert, welcher bei einem Synchronisationsimpuls in das
    // Z�hlerregister (TBCTR) geladen wird. Falls der PWM-Takt
    // gleich der Z�hler-Takt ist (CLKDIV = 1, HSPCLKDIV = 1 ->
    // EPWMCLK = TBCLK), wird der Wert aus dem Phasen-Register (TBPHS)
    // 2 Taktzyklen nach dem Synchronisationsimpuls in das Z�hler-
    // Register geladen. Falls der Z�hlertakt kleiner als der PWM-Takt
    // ist (CLKDIV > 1 und/oder HSPCLKDIV > 1 -> EPWMCLK > TBCLK),
    // betr�gt die Verz�gerung 1 Taktzyklus. Im aktuellen Datenblatt
    // (Reference Manual TMS320F2838x) steht dieses Verhalten nicht
    // beschrieben, es ist jedoch z.B. auf S. 1783 im Reference Manual
    // TMS320F280049C dokumentiert. Zudem konnte diese Verhalten
    // nachgewiesen werden. Im Up-Modus kann diese Verz�gerung durch
    // TBPHS = 2 bzw. TBPHS = 1 kompensiert werden. Im Up-Down-Modus
    // funktioniert das nicht, weil das Slave-PWM-Modul ggf. runter
    // z�hlt w�hrend der Synchronisationsimpuls kommt. In diesem Fall
    // w�rde der Z�hler z.B. von 0 auf 2 bzw. 1 zur�ckspringen und von
    // dort aus wieder runtergez�hlt werden. F�r diesen Betriebsfall
    // kann das Bit TBCTL.bit.PHSDIR verwendet werden. Dieses Bit ist
    // nur wirksam, falls der Up-Dowm-Modus akiv ist. Es legt die Z�hl-
    // richtung des PWM-Moduls fest, nachdem ein Synchronisationsevent
    // aufgetreten ist und ein Wert aus dem Register TBPHS geladen wurde.
    // Alternative: Zwei Compare-Register verwenden (CMPA und CMPB).
    // Beim Hochz�hlen wird dann beispielsweise der Ausgang auf 1
    // umgeschaltet, wenn TBCTR = CMPA und beim Runterz�hlen auf 0, wenn
    // TBCTR = CMPB. Die CMPA-Register der Slave-PWM-Module werden um
    // den Wert der Verz�gerungszeit (2 oder 1) reduziert und die CMPB-
    // Register um den Wert (2 oder 1) erh�ht
    EPwm3Regs.TBPHS.bit.TBPHS = 0;
    // Wert, welcher in TBPRD geschrieben wird, sofort �bernehmen
    EPwm3Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periode f�r 16 kHz Schaltfrequenz setzen
    // bei 100 MHz EPWMCLK und Auf-Abz�hlmodus
    EPwm3Regs.TBPRD = PWM_PERIOD;
    // Betriebsart: hoch-runter z�hlen
    EPwm3Regs.TBCTL.bit.CTRMODE = PWM_TB_COUNT_UPDOWN;
    // Compare-Register erst beschreiben, wenn der Z�hler den Wert 0 erreicht
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = PWM_CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = PWM_CC_SHDW_CTR_ZERO;
    // Tastverh�ltnis auf 0 setzen
    // Dieser Wert wird kontinuierlich mit TBCTR (Timer-Z�hlwert) verglichen.
    // Sind beide Werte gleich, wird abh�nging von AQCTLA der PWM3A-Pin gesetzt
    EPwm3Regs.CMPA.bit.CMPA = 0;
    // PWM3A-Pin auf high ziehen, wenn TBCTR (Timer-Z�hlwert) beim hochz�hlen den Wert CMPA erreicht
    EPwm3Regs.AQCTLA.bit.CAU = PWM_AQ_SET;
    // PWM3A-Pin auf low ziehen, wenn TBCTR (Timer-Z�hlwert) beim runterz�hlen den Wert CMPB erreicht
    EPwm3Regs.AQCTLA.bit.CBD = PWM_AQ_CLEAR;
    // Totzeiten (siehe S. 2898 Reference Manual TMS320F2838x):
    // An GPIO4 wird das PWM3A-Signal mit einer um DBRED verz�gerten steigenden
    // Fanke ausgegeben. An GPIO5 wird das invertierte PWM3A-Signal mit einer
    // um DBFED verz�gerten fallenden Flanke ausgegeben. Somit h�ngt das Signal
    // an GPIO5 direkt von PWM3A ab und der Kanal B (CMPB und AQCTLB) muss nicht
    // initialisiert werden.
    // Totzeit-Z�hler l�uft mit der selben Frequenz wie der PWM-Z�hler
    EPwm3Regs.DBCTL.bit.HALFCYCLE = 0;
    // Das Eingangssignal f�r beide Totzeit-Z�hler ist PWM3A
    // (steigende und fallende Flanke)
    EPwm3Regs.DBCTL.bit.IN_MODE = PWM_DB_IN_A_ALL;
    // PWM3A-Signal nach Totzeitverz�gerung steigende Flanke nicht invertieren,
    // PWM3A-Signal nach Totzeitverz�gerung fallende Flanke invertieren
    EPwm3Regs.DBCTL.bit.POLSEL = PWM_DB_POL_B_INV;
    // PWM3A-Ausgangssignal: PWM3A-Signal mit verz�gerter steigender Flanke
    // PWM3B-Ausgangssignal: PWM3A-Signal mit verz�gerter fallender Flanke, invertiert
    EPwm3Regs.DBCTL.bit.OUT_MODE = PWM_DB_NONE_BYPASSED;
    // Totzeit f�r steigende Flanke
    EPwm3Regs.DBRED.bit.DBRED = PWM_DEAD_BAND;
    // Totzeit f�r fallende Flanke
    EPwm3Regs.DBFED.bit.DBFED = PWM_DEAD_BAND;
    // Z�hler auf 0 setzen
    EPwm3Regs.TBCTR = 0;

    // Synchronisierungstakt einschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    // PWM-Module einmalig per Software synchronisieren
    EPwm1Regs.TBCTL.bit.SWFSYNC = 1;

    // ePWM1A-Signal auf GPIO 145 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPELOCK.bit.GPIO145 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPEGMUX2.bit.GPIO145 = (1 >> 2);
    GpioCtrlRegs.GPEMUX2.bit.GPIO145  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPEPUD.bit.GPIO145 = 1;
    // ePWM1B-Signal auf GPIO 146 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPELOCK.bit.GPIO146 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPEGMUX2.bit.GPIO146 = (1 >> 2);
    GpioCtrlRegs.GPEMUX2.bit.GPIO146  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPEPUD.bit.GPIO146 = 1;

    // ePWM2A-Signal auf GPIO 147 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPELOCK.bit.GPIO147 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPEGMUX2.bit.GPIO147 = (1 >> 2);
    GpioCtrlRegs.GPEMUX2.bit.GPIO147  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPEPUD.bit.GPIO147 = 1;
    // ePWM2B-Signal auf GPIO 148 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPELOCK.bit.GPIO148 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPEGMUX2.bit.GPIO148 = (1 >> 2);
    GpioCtrlRegs.GPEMUX2.bit.GPIO148  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPEPUD.bit.GPIO148 = 1;

    // ePWM3A-Signal auf GPIO 149 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPELOCK.bit.GPIO149 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPEGMUX2.bit.GPIO149 = (1 >> 2);
    GpioCtrlRegs.GPEMUX2.bit.GPIO149  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPEPUD.bit.GPIO149 = 1;
    // ePWM3B-Signal auf GPIO 150 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPELOCK.bit.GPIO150 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPEGMUX2.bit.GPIO150 = (1 >> 2);
    GpioCtrlRegs.GPEMUX2.bit.GPIO150  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPEPUD.bit.GPIO150 = 1;

		// Register-Schreibschutz setzen
		EDIS;
}


//=== Function: Pwm1ISR ===========================================================================
///
/// @brief  ISR wird aufgerufen wenn der Z�hler des ePMW1-Moduls den Wert 0 erreicht
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void Pwm1ISR(void)
{
		// Hier k�nnen die Tastverh�ltnisse f�r den n�chsten Schaltvorgang
		// f�r jede der drei Halbbr�cken gesetzt werden
		EPwm1Regs.CMPA.bit.CMPA = 400;
		// ePWM2 und ePMW3 werden mit ePWM1 synchronisiert:
		// Synchronisationsverz�gerung kompensieren (siehe PwmInitPwm123())
		EPwm2Regs.CMPA.bit.CMPA = 600 - PWM_SYNCHRONIZAION_DELAY;
		EPwm2Regs.CMPB.bit.CMPB = 600 + PWM_SYNCHRONIZAION_DELAY;
		EPwm3Regs.CMPA.bit.CMPA = 800 - PWM_SYNCHRONIZAION_DELAY;
		EPwm3Regs.CMPB.bit.CMPB = 800 - PWM_SYNCHRONIZAION_DELAY;

    // Interrupt-Flag im ePWM1-Modul l�schen
		EPwm1Regs.ETCLR.bit.INT = 1;
    // Interrupt der Gruppe 3 best�tigen (da geh�rt der ePWM1-Interrupt zu)
    PieCtrlRegs.PIEACK.bit.ACK3 = 1;
}


//=== Function: PwmInitPwm8 =======================================================================
///
/// @brief  Funktion initialisiert das ePWM8-Modul um alle 100 ms einen Interrupt auszul�sen
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void PwmInitPwm8(void)
{
    // Register-Schreibschutz aufheben
		EALLOW;
    // Synchronisierungstakt w�hrend der Konfiguration ausschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    // Takt f�r das PWM8-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR2.bit.EPWM8 = 1;
    __asm(" RPT #4 || NOP");
		// Z�hlrichtung des Timers: hoch
    EPwm8Regs.TBCTL.bit.CTRMODE = PWM_TB_COUNT_UP;
		// Taktteiler: 1280
    // TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)
    // EPWMCLK = SYSCLK / 2 = 100 MHz
    // (siehe "DeviceInit()" und S. 165 Reference Manual TMS320F2838x)
    EPwm8Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_128;
		EPwm8Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_10;
		// TBCTR nicht mit Wert aus dem Phasenregister laden
    EPwm8Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_DISABLE;
    // Daten, welche in das TBPRD Register (Periodendauer) geschrieben
    //  werden, direkt laden (ohne Umweg �ber das Shadow-Register)
    EPwm8Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periodendauer auf 100 ms setzen
    // Periodendauer = EPWMCLK / (HSPCLKDIV * CLKDIV * (TBPRD+1))
    EPwm8Regs.TBPRD = 7811;
		// Z�hler auf 0 setzen
		EPwm8Regs.TBCTR = 0;
		// PWM8-Interrupt einschalten
		EPwm8Regs.ETSEL.bit.INTEN = 1;
    // Interrupt ausl�sen, wenn der Timer den Endwert (TBPRD) erreicht hat
		EPwm8Regs.ETSEL.bit.INTSEL = PWM_ET_CTR_PRD;
    // ISR aufrufen, sobald 1 Interrupt ausgel�st wurde
		EPwm8Regs.ETPS.bit.INTPRD = PWM_ET_1ST;

    // Interrupt-Service-Routinen f�r den ePWM8-Interrupt an die
    // entsprechende Stelle (ePWM8_INT) der PIE-Vector Table speichern
    PieVectTable.EPWM8_INT = &Pwm8ISR;
    // INT3.8-Interrupt freischalten (Zeile 3, Spalte 8 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER3.bit.INTx8 = 1;
    // CPU-Interrupt 3 einschalten (Zeile 3 der Tabelle)
    IER |= M_INT3;

    // Synchronisierungstakt wieder einschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
		// Register-Schreibschutz setzen
		EDIS;
}


//=== Function: Pwm8ISR ===========================================================================
///
/// @brief  ISR wird aufgerufen, sobald ePWM8 einen �berlauf hat. Kann als
///					Zeitgeber f�r periodisch durchzuf�hrende Aufgaben verwendet werden
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void Pwm8ISR(void)
{
		// Hier k�nnen Flags f�r periodisch
		// durchzuf�hrende Operationen gesetzt werden

    // Interrupt-Flag im ePWM8-Modul l�schen
		EPwm8Regs.ETCLR.bit.INT = 1;
    // Interrupt-Flag der Gruppe 3 l�schen (da geh�rt der ePMW8-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK3 = 1;
}



