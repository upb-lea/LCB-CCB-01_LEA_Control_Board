//=================================================================================================
/// @file       myTripzone.c
///
/// @brief      Datei enth�lt Variablen und Funktionen um die Funktion der Tripzone der PWM-Module
///							und die der internen Komparatoren der analogen Eingangspins zu demonstrieren. Das
///							Signal vom Analogeingang A2 wird auf die beiden nichtinvertierenden Eing�nge der
///							beiden Komparatoren vom Komparatorsubsystem 1 (CMPSS1) gelegt. Auf die beiden
///							invertierenden Eing�nge wird jeweils die (einstellbare) Spannung eines DACs gelegt.
///							Die Ausg�nge der Komparatoren werden �ber die ePWM X-bar auf das Digital-Compare-
///							Modul des ePWM1-Moduls gelegt und l�sen dort zwei Events aus (DCAEVT1 und DCBEVT1).
///							Diese Events werden genutzt, um in dem Tripzone-Modul die Zust�nde der PWM-Pins
///							zu �bersteuern. Liegt die Eingangsspannung an Pin A2 unter- oder oberhalb eines
///							bestimmten Werts (DACLVALS bzw. DACHVALS), so liegen beide PWM-Pins auf Low-Pegel.
///							Wenn die Spannung innerhalb dieser Grezen liegt, werden die Pins mit einem Recht-
///							ecksignal angesteuert (PWM-Signal). Beim Auftreten der Events (Komparatoren
///							schalten auf 1 oder 0 um), kann ein Interrupt ausgel�st werden. Zus�tzlich l�sst
///							sich der ausl�sende Komparator in einem Flag-Register identifizieren. Auf diese
///							Weise kann softwareunabh�ngig ein Strom �berwacht und im Falle eines zu hohen
/// 						Werts die Schalter eines Wechselrichter o.�. sehr schnell abgeschaltet werden.
///
///							Verkabelung:
///							- ADCINA2/CMPIN1P an Mittelabgriff Poti (0 ... 3,3 V)
///							- GPIO 0 und 1 an Oszilloskop
///
/// @version    V1.0
///
/// @date       08.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myTripzone.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: TripzoneInitCmpss1 ================================================================
///
/// @brief  Funktion initialisiert die beiden High- und Low-Komparatoren des Analog-Eingangs A2
///					und die zugeh�rigen DACs (Analogsignal jeweils am +Eingang, DACs jeweils am -Eingang
///					der Komparatoren, DAC mt VDDA als Referenz, Komparatorausg�nge auf X-Bar legen), die
///					ePWM X-Bar und das ePWM1-Modul. Wenn die analoge Spannung an Pin A2 unterhalb von
///					(1000/4095) * 3,3 V (DACLVALS) oder oberhalb von (3000/4095) * 3,3 V (DACHVALS)
///					liegt, werden beide PWM-Pins (ePWM1A und ePWM1B) auf Low-Pegel gesetzt. Anderfalls
///					liegt an den Pins ein PWM-Signal mit 50 % Tastverh�ltnis und 10 kHz Freuqenz an
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void TripzoneInitCmpss1(void)
{
		// Register-Schreibschutz aufheben
		EALLOW;

		// Takt f�r das CMPSS1-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
		CpuSysRegs.PCLKCR14.bit.CMPSS1 = 1;
		__asm(" RPT #4 || NOP");
		// �berblick Komparator-Subsystem siehe S. 2505 Reference Manual TMS320F2838x.
		// Komparatorsystem 1 einschalten
		// 0: Komparator ausgeschaltet
		// 1: Komparator eingeschaltet
		Cmpss1Regs.COMPCTL.bit.COMPDACE = 1;
		// VDDA als Referenzspannung f�r die DACs setzen
		// 0: VDDA als Referenzspannung (3,3 V Analog Spannungsversorgung)
		// 1: VDAC als Referenzspannung (ADCINB0)
		Cmpss1Regs.COMPDACCTL.bit.SELREF = 0;
		// Wert des DAC aus dem Register DACHVALS �bernehmen.
		// Nur f�r High-Komparator DAC, Low-Komparator DAC wird immer �ber
		// DACHVALS gespeist (siehe S. 2745 Reference Manual TMS320F2838x)
		// 0: DAC-Wert wird gesetzt durch Register DACHVALS
		// 1: DAC-Wert wird gesetzt vom Rampengenerator
		Cmpss1Regs.COMPDACCTL.bit.DACSOURCE = 0;
		// Wert aus dem Shadow-Register DACHVALS bei jedem SYSCLK �bernehmen
		// 0: Wert bei jedem SYSCLK �bernehmen
		// 1: Wert bei EPWMSYNCPER �bernehmen
		Cmpss1Regs.COMPDACCTL.bit.SWLOADSEL = 0;

		// High-Komparator konfigurieren:
		// DAC an den invertierenden Eingang des Komparators anlegen
		// 0: DAC ist an invertierenden Eingang
		// 1: Externer Pin ist an invertierenden Eingang
		Cmpss1Regs.COMPCTL.bit.COMPHSOURCE = 0;
		// Wert DAC setzen (12 Bit Shadow-Register).
		// Wert wird abh�ngig vom Bit COMPDACCTL.SWLOADSEL �bernommen
		Cmpss1Regs.DACHVALS.bit.DACVAL = 3000;
		// Ausgang des Komparators nicht invertieren
		// 0: Ausgang nicht invertieren
		// 1: Ausgang invertieren
		Cmpss1Regs.COMPCTL.bit.COMPHINV = 0;
		// Ausgang CTRIPx   : ePWM X-Bar
		// Ausgang CTRIPOUTx: Output X-Bar
		// Ausgangssignal des Komparators direkt auf die ePWM X-Bar
		// legen (siehe S. 2745 Reference Manual TMS320F2838x)
		// 0: Komparator-Ausgangssignal asynchron zu SYSCLK (ohne
		//    zeitliche Verz�gerung) zu der ePWM X-Bar f�hren
		// 1: Komparator-Ausgangssignal synchron zu SYSCLK zu
		//    der ePWM X-Bar f�hren
		// 2: Gefiltertes Komparator-Ausgangssignal zu der
		//    ePWM X-Bar f�hren
		// 3: Gefiltertes Komparator-Ausgangssignal �ber ein
		//    Flip-Flop zu der ePWM X-Bar f�hren
		Cmpss1Regs.COMPCTL.bit.CTRIPHSEL = 0;

		// Low-Komparator konfigurieren:
		// DAC an den invertierenden Eingang des Komparators anlegen
		Cmpss1Regs.COMPCTL.bit.COMPLSOURCE = 0;
		// Wert DAC setzen (12 Bit)
		Cmpss1Regs.DACLVALS.bit.DACVAL = 1000;
		// Ausgang des Komparators nicht invertieren
		Cmpss1Regs.COMPCTL.bit.COMPLINV = 0;
		// Ausgangssignal des Komparators direkt zu der ePWM X-Bar f�hren
		Cmpss1Regs.COMPCTL.bit.CTRIPLSEL = 0;

		// ePWMX-Bar konfigurieren:
		// Die Signale der Komparatorsubsysteme in die ePWM X-Bar fallen unter
		// "Other Sources" (siehe S. 2142 Reference Manual TMS320F2838x).
		// Quellen f�r die Output und ePWM X-Bar sind Trip 4, 5, 7, 8, 9,
		// 10, 11, 12 (Trip 1, 2, 3, 6 gehen direkt in die ePWM-Module).
    // Die ePWM X-Bar hat 8 Ausg�nge, die jeweils bis zu 32 Signale
    // f�hren k�nnen. Jeder der 32 Multiplexer (MUX0 ... MUX31) kann
    // aus bis zu 4 Signalen w�hlen. Je nach gew�nschtem Signal muss
    // der entsprechende Multiplexer ausgew�hlt werden. Anschlie�end
    // muss das Ausgangssignal des Multiplexers noch durchgeschleift
    // werden. Optional kann das aus den 32 Signalquellen veroderte
    // Gesamtsignal noch invertiert werden (siehe S. 2144 Reference
    // Manual TMS320F2838x).
		// Ausgangssignal vom CMPSS1 High-Komparator auf TRIP4
		// legen (nach der Tabelle geht das nur auf MUX0)
		EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX0 = 0;
		// Jedes Eingangssignal muss neben einer Quellzuweisung
		// zus�tzlich freigeschaltet werden
		// Noch ggf. aktive Signalquellen sperren
		EPwmXbarRegs.TRIP4MUXENABLE.all = 0;
		// Eingangssignal f�r MUX0 freigeben
		// 0: Signalquelle freigegeben
		// 1: Signalquelle gesperrt
		EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX0 = 1;
		// Den Ausgang der ePWM X-Bar (= TRIP-Eingang
		// der ePWM-Module) nicht invertieren
		// 0: Signal nicht invertiert (active-high)
		// 1: Signal invertiert (active-low)
		EPwmXbarRegs.TRIPOUTINV.bit.TRIP4 = 0;
		// Ausgangssignal vom CMPSS1 Low-Komparator auf TRIP5
		// legen (nach der Tabelle geht das nur auf MUX1)
		EPwmXbarRegs.TRIP5MUX0TO15CFG.bit.MUX1 = 0;
		// Noch ggf. aktive Signalquellen sperren
		EPwmXbarRegs.TRIP5MUXENABLE.all = 0;
		// Eingangssignal f�r MUX1 freigeben
		EPwmXbarRegs.TRIP5MUXENABLE.bit.MUX1 = 1;
		// Ausgang der ePWM X-Bar nicht invertieren
		EPwmXbarRegs.TRIPOUTINV.bit.TRIP5 = 0;

		// PWM-Modul konfigurieren:
		// Takt f�r das ePWM1-Modul einschalten und 5 Takte
		// warten, bis der Takt zum Modul durchgestellt ist
		// (siehe S. 169 Reference Manual TMS320F2838x)
		CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
		__asm(" RPT #4 || NOP");
		// TRIPIN4 (CMPSS1 High-Komparator Ausgangssignal von der
		// ePWM X-Bar) als Quelle f�r das DCAH-Signal verwenden
		// (siehe S. 2917 Reference Manual TMS320F2838x)
		EPwm1Regs.DCTRIPSEL.bit.DCAHCOMPSEL = PWM_DC_TRIP_TRIPIN4;
		// TRIPIN5 (CMPSS1 Low-Komparator Ausgangssignal von der
		// ePWM X-Bar) als Quelle f�r das DCBH-Signal verwenden
		EPwm1Regs.DCTRIPSEL.bit.DCBHCOMPSEL = PWM_DC_TRIP_TRIPIN5;
		// Event DCAEVT1 ausl�sen, wenn das DCAH-Signal high ist
		// 0: Event ausgeschaltet
		// 1: Event wird ausgel�st, wenn DCxH = low
		// 2: Event wird ausgel�st, wenn DCxH = high
		// 3: Event wird ausgel�st, wenn DCxL = low
		// 4: Event wird ausgel�st, wenn DCxL = high
		// 5: Event wird ausgel�st, wenn DCxH = low & DCxL = high
		EPwm1Regs.TZDCSEL.bit.DCAEVT1 = PWM_DC_DCXH_HIGH;
		// Event DCBEVT1 ausl�sen, wenn das DCBH-Signal low ist
		EPwm1Regs.TZDCSEL.bit.DCBEVT1 = PWM_DC_DCXH_LOW;
		// Ungefiltertes/originales DCAEVT1-Signal f�r die weitere Verarbeitung
		// verwenden (siehe S. 2921 + 2922 Reference Manual TMS320F2838x)
		// 0: Originales DCAEVT1-Signal verwenden
		// 1: Gefiltertes DCAEVT1-Signal verwenden
		EPwm1Regs.DCACTL.bit.EVT1SRCSEL = PWM_DC_RAW_EVENT;
		// Ungefiltertes/originales DCBEVT1-Signal f�r die weitere Verarbeitung
		EPwm1Regs.DCBCTL.bit.EVT1SRCSEL = PWM_DC_RAW_EVENT;
		/*
		// Interrupt ausl�sen, wenn das DCAEVT1-Event auftritt
		// 0: Interrupt auschgeschaltet
		// 1: Interrupt eingeschaltet
		EPwm1Regs.TZEINT.bit.DCAEVT1 = PWM_DC_INT_ENABLE;
		// Interrupt ausl�sen, wenn das DCBEVT1-Event auftritt
		EPwm1Regs.TZEINT.bit.DCBEVT1 = PWM_DC_INT_ENABLE;
		// Interrupt ausl�sen, wenn ein OST-Event auftritt
		EPwm1Regs.TZEINT.bit.OST = PWM_DC_OST_INT_ENABLE;
		// F�r jedes Event wird immer der gleiche Interrupt ausgel�st (EPWMxTZINT).
		// CPU-Interrupts w�hrend der Konfiguration global sperren
		DINT;
		// Interrupt-Service-Routinen f�r den TZ-Interrupt an die
		// entsprechende Stelle (EPWM1_TZ_INT) der PIE-Vector Table speichern
		PieVectTable.EPWM1_TZ_INT = &TripzonePwm1ISR;
		// EPWM1-TZ-Interrupt freischalten (Zeile 2, Spalte 1 der Tabelle)
		// (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
		PieCtrlRegs.PIEIER2.bit.INTx1 = 1;
		// CPU-Interrupt 2 einschalten (Zeile 2 der Tabelle)
		IER |= M_INT2;
		// CPU-Interrupts nach Konfiguration global wieder freigeben
		EINT;
		*/
		// F�r das Ausl�sen einer PWM-Schaltaktion (Signal DCAEVT1.force) das
		// DCAEVT1-Signal asynchron zu SYSCLK verwenden (keine Zeitverz�gerung)
		// (siehe S. 2921 Reference Manual TMS320F2838x)
		// 0: Signal DCAEVT1.force synchron zu SYSCLK
		// 1: Signal DCAEVT1.force asynchron zu SYSCLK
		EPwm1Regs.DCACTL.bit.EVT1FRCSYNCSEL = PWM_DC_EVENT_ASYNC;
		// F�r das Ausl�sen einer PWM-Schaltaktion (Signal DCBEVT1.force)
		// das DCBEVT1-Signal asynchron zu SYSCLK verwenden (keine Zeitverz�gerung)
		// 0: Signal asynchron zu SYSCLK
		// 1: Signal synchron zu SYSCLK
		EPwm1Regs.DCBCTL.bit.EVT1FRCSYNCSEL = PWM_DC_EVENT_ASYNC;
		// F�r das Ausl�sen einer PWM-Schaltaktion (Signal DCAEVT1.force) das
		// DCAEVT1-Signal direkt verwenden (Flip-Flop umgehen). Die Information
		// zum Einstellen des Nultiplexers fehlt im Reference Manual! Ich habe
		// diese Information �ber eine Anfrage bei TI erhalten (Mail: 22.04.2022)
		// 0: Signal direkt
		// 1: Signal �ber Flip-Flop
		EPwm1Regs.DCACTL.bit.EVT1LATSEL = PWM_DC_EVENT_UNLATCHED;
		// F�r das Ausl�sen einer PWM-Schaltaktion (Signal DCBEVT1.force)
		// das DCAEVT1-Signal direkt verwenden (Flip-Flop umgehen)
		EPwm1Regs.DCBCTL.bit.EVT1LATSEL = PWM_DC_EVENT_UNLATCHED;
		// Der Pegel am PWM-Ausgangspin des Mikrocontrollers wird durch vier
		// Signale bestimmt (siehe S. 2910 Reference Manual TMS320F2838x):
		// 1: TZx (Cycle-by-Cycle und One-Shot Trip Events)
		// 2: DCxEVENT1
		// 3: DCxEVENT2
		// 4: EPWMx
		// Die h�chste Priorit�t hat das erste Signal (1), die kleinste das letzte (4)
		// (siehe S. 2920 Reference Manual TMS320F2838x, force signal). Jedes Signal
		// mit einer h�heren Priotit�t �bersteuert Signale mit einer niedrigeren
		// Priorit�t. Das Signal EPWMxA/B wird durch die Konfiguration des Timers etc.
		// bestimmt und stellt das eigendliche PWM-Signal dar. Dieses kann von Tripzone
		// -Signalen �bersteuert werden (typischerweise im Fehlerfall um in einen sicheren
		// Zustand zu wechseln). Das Verhalten der PWM-Pins durch die Tripzone-Signale
		// wird entweder durch das Register TZCTL (TZCTL2.ETZE = 0) oder durch die
		// Register TZCTL2, TZCTLDCA und TZCTLDCB festgelegt (TZCTL2.ETZE = 1). Letzteres
		// erlaubt eine Konfiguration abh�ngig von der Z�hlrichtung des Timers (hoch/runter).
		// Tripzone-Einstellungen aus dem TZCTL-Register benutzen
		// 0: Tripzone-Einstellungen aus dem TZCTL-Register benutzen
		// 1: Tripzone-Einstellungen aus dem TZCTL2-, TZCTLDCA- und TZCTLDCB-Register benutzen
		EPwm1Regs.TZCTL2.bit.ETZE = PWM_TZ_CONFIG_BY_TZCTL;
		/*
		// Bei einem DCAEVT1-Event den ePWM1A-Pin auf Low-Pegel setzen.
		// Bei diesem Betrieb werden die PWM-Pins solange auf dem konfigurierten Zustand
		// gehalten, solange das DCxEVTx-Event anliegt. Nachdem das DCxEVTx-Signal in den
		// Ruhezustand wechselt, �bernehmen die ePWMx-Signale wieder die Kontrolle �ber
		// die PWM-Pins. Die Flags f�r die ausgel�sten Events bleiben trotzdem gesetzt
		// und m�ssen manuell gel�scht werden.
		// 0: PWM-in auf High-Impedance setzen
		// 1: PWM-Pin auf High-Pegel setzen
		// 2: PWM-Pin auf Low-Pegel setzen
		// 3: Keine Aktion durchf�hren
		EPwm1Regs.TZCTL.bit.DCAEVT1 = PWM_TZ_FORCE_LO;
		// Bei einem DCBEVT1-Event den ePWM1B-Signal auf 0 setzen
		EPwm1Regs.TZCTL.bit.DCBEVT1 = PWM_TZ_FORCE_LO;
		*/
		// DCAEVT1-Signal als One-Shot-Trip Quelle setzen.
		// Bei diesem Betrieb verbleiben die PWM-Pins auf dem konfigurierten Zustand,
		// bis das One-Shot-Flag gel�scht wird (TZCLR.OST), auch wenn das ausl�sende
		// Event (TZ1...6, DCxEVT1) bereits verstrichen ist (siehe Trip-Zone Submodule
		// Mode Control Logic S. 2910 Reference Manual TMS320F2838x). Dieser Betrieb
		// (One-Shot-Trip) �bersteuert die Signale DCxEVT1.force und DCxEVT2.force. Die
		// Flags der DCxEVT1- und DCxEVT2 werden ebenfalls gesetzt und m�ssen manuell
		// gel�scht werden.
		// 0: DCAEVT1 als Quelle deaktiviert
		// 1: DCAEVT1 als Quelle aktiviert
		EPwm1Regs.TZSEL.bit.DCAEVT1 = PWM_TZ_ENABLE;
		// DCBEVT1-Signal als One-Shot-Trip Quelle setzen
		EPwm1Regs.TZSEL.bit.DCBEVT1 = PWM_TZ_ENABLE;
		// Bei einem TZA-Event den ePWM1A-Pin auf Low-Pegel setzen
		// 0: PWM-in auf High-Impedance setzen
		// 1: PWM-Pin auf High-Pegel setzen
		// 2: PWM-Pin auf Low-Pegel setzen
		// 3: Keine Aktion durchf�hren
		EPwm1Regs.TZCTL.bit.TZA = PWM_TZ_FORCE_LO;
		// Bei einem TZB-Event den ePWM1B-Pin auf Low-Pegel setzen
		EPwm1Regs.TZCTL.bit.TZB = PWM_TZ_FORCE_LO;

		// Nicht Tripzone-spezifische PWM-Konfiguration:
    // Takt-Teiler des PWM-Moduls setzen
    // PWM-Takt = SYSCLKOUT / (CLKDIV * HSPCLKDIV)
    EPwm1Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_1;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_1;
    // TBCTR nicht mit Wert aus dem Phasenregister laden
    EPwm1Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_DISABLE;
    // Betriebsart: hoch-runter z�hlen
    EPwm1Regs.TBCTL.bit.CTRMODE = PWM_TB_COUNT_UPDOWN;
    // Wert, welcher in TBPRD geschrieben wird, sofort �bernehmen
    EPwm1Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periode f�r 10 kHz Schaltfrequenz setzen
    EPwm1Regs.TBPRD = 5000;
    // Compare-Register erst beschreiben, wenn der Z�hler den Wert 0 erreicht
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = PWM_CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = PWM_CC_SHDW_CTR_ZERO;
    // Tastverh�ltnis auf 50 % setzen
    // Dieser Wert wird kontinuierlich mit TBCTR (Timer-Z�hlwert) verglichen.
    // Sind beide Werte gleich, wird abh�nging von AQCTLA der PWM1A-Pin gesetzt
    EPwm1Regs.CMPA.bit.CMPA = 2500;
    // PWM1A-Pin auf high ziehen, wenn TBCTR (Timer-Z�hlwert) beim hochz�hlen den Wert CMPA erreicht
    EPwm1Regs.AQCTLA.bit.CAU = PWM_AQ_SET;
    // PWM1A-Pin auf low ziehen, wenn TBCTR (Timer-Z�hlwert) beim runterz�hlen den Wert CMPA erreicht
    EPwm1Regs.AQCTLA.bit.CAD = PWM_AQ_CLEAR;
    // Totzeiten (siehe Figure 18-33, Reference Manual S. 1853):
    // An GPIO0 wird das PWM1A-Signal mit einer um DBRED verz�gerten steigenden
    // Fanke ausgegeben. An GPIO1 wird das invertierte PWM1A-Signal mit einer
    // um DBFED verz�gerten fallenden Flanke ausgegeben Somit h�ngt das Signal
    // an GPIO1 direkt von PWM1A ab und der Kanal B (CMPB und AQCTLB) muss nicht
    // initialisiert werden.
    // Totzeit-Z�hler  mit der selben Frequenz wie der PWM-Z�hler laufen lassen
    // 0: CLK Totzeit-Z�hler = TBCLK
    // 1: CLK Totzeit-Z�hler = TBCLK / 2
    EPwm1Regs.DBCTL.bit.HALFCYCLE = PWM_DB_FULL_CYCLE;
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
    EPwm1Regs.DBRED.bit.DBRED = 50;
    // Totzeit f�r fallende Flanke
    EPwm1Regs.DBFED.bit.DBFED = 50;
    // Timer auf 0 setzen
    EPwm1Regs.TBCTR = 0;
    // Synchronisierungstakt einschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;

    // GPIOs konfigurieren:
    // ePWM1A-Signal auf GPIO 0 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO0 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = (1 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO0  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;
    // ePWM1B-Signal auf GPIO 1 legen:
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO1 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = (1 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO1  = (1 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;

		// Register-Schreibschutz setzen
		EDIS;
}


//=== Function: TripzonePwm1ISR ==================================================================
///
/// @brief  ISR wird aufgerufen, wenn ein Tripzone-Event des ePWM1-Moduls
///					auftritt und der zugeh�rige Interrupt eingeschaltet ist
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void TripzonePwm1ISR(void)
{
		// Trip-Flag des DCAEVT1-Events l�schen
		if (EPwm1Regs.TZFLG.bit.DCAEVT1)
		{
				EPwm1Regs.TZCLR.bit.DCAEVT1 = 1;
		}
		// Trip-Flag des DCBEVT1-Events l�schen
		if (EPwm1Regs.TZFLG.bit.DCBEVT1)
		{
				EPwm1Regs.TZCLR.bit.DCBEVT1 = 1;
		}
		// Trip-Flag des OST-Events l�schen
		if (EPwm1Regs.TZFLG.bit.OST)
		{
				EPwm1Regs.TZCLR.bit.OST = 1;
		}

		// Allgemeines Trip-Flag l�schen (wird immer mitgesetzt, wenn ein
		// Trip-Flag gesetzt wird und der zugeh�rige Interrupt eingeschaltet
		// ist (siehe S. 2911 Reference Manual TMS320F2838x)
		EPwm1Regs.TZCLR.bit.INT = 1;
		// Interrupt-Flag der Gruppe 2 l�schen (da geh�rt der EPWM1-TZ-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK2 = 1;
}

