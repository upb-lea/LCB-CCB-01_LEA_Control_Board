//=================================================================================================
/// @file       myPWM.c
///
/// @brief      Datei enthält Variablen und Funktionen um die Funktion der ePWM-Module zu
///							demonstrieren. Das ePWM8-Modul wird so initialisiert, dass alle 100 ms ein
///							Interrupt ausgelöst wird und so als Zeitgeber für periodisch zu erledigende
///							Aufgaben genutzt werden kann.
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
//=== Function: PwmInitPwm8 =======================================================================
///
/// @brief  Funktion initialisiert das ePWM8-Modul um alle 100 ms einen SOCA-Trigger
///					auszulösen um eine ADC-messung zu starten
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
    // Synchronisierungstakt während der Konfiguration ausschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    // Takt für das PWM8-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR2.bit.EPWM8 = 1;
    __asm(" RPT #4 || NOP");
		// Zählrichtung des Timers: hoch
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
    //  werden, direkt laden (ohne Umweg über das Shadow-Register)
    EPwm8Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periodendauer auf 100 ms setzen
    // Periodendauer = EPWMCLK / (HSPCLKDIV * CLKDIV * (TBPRD+1))
    EPwm8Regs.TBPRD = PWM_SOCA_TRIGGER_PERIOD;
		// Zähler auf 0 setzen
		EPwm8Regs.TBCTR = 0;
		// Konfiguration des PWM-Moduls zum Triggern einer ADC-Messung:
		// SOCA-Trigger einschalten
    // 0: SOCA-Trigger ausgeschaltet
    // 1: SOCA-Trigger eingeschaltet
		EPwm8Regs.ETSEL.bit.SOCAEN = PWM_ET_SOC_ENABLE;
		// SOCA-Trigger erzeugen, wenn der PWM-Zähler (TBCTR) den Wert 0 erreicht
		// 0: Trigger ausgeschaltet
    // 1: Trigger wenn TBCTR = 0
    // 2: Trigger wenn TBCTR = TBPRD
    // 3: Trigger wenn TBCTR = TBPRD oder TBCTR = 0
    // 4: Trigger wenn TBCTR = CMPA (SOCASELCMP = 0) bzw. TBCTR = CMPC (SOCASELCMP = 1) beim Hochzählen
    // 5: Trigger wenn TBCTR = CMPA (SOCASELCMP = 0) bzw. TBCTR = CMPC (SOCASELCMP = 1) beim Runterzählen
    // 6: Trigger wenn TBCTR = CMPB (SOCASELCMP = 0) bzw. TBCTR = CMPD (SOCASELCMP = 1) beim Hochzählen
    // 7: Trigger wenn TBCTR = CMPB (SOCASELCMP = 0) bzw. TBCTR = CMPD (SOCASELCMP = 1) beim Runterzählen
		EPwm8Regs.ETSEL.bit.SOCASEL = PWM_ET_CTR_ZERO;
		// SOCA-Trigger bei jedem Event (siehe ETSEL.SOCASEL) erzeugen
		// 0: Event-Trigger ausgeschaltet
		// 1: SOCA-Trigger beim 1. Auftreten des Events auslösen
		// 2: SOCA-Trigger beim 2. Auftreten des Events auslösen
		// n: SOCA-Trigger beim n. Auftreten des Events auslösen
		EPwm8Regs.ETPS.bit.SOCAPRD = PWM_ET_1ST;

    // Synchronisierungstakt wieder einschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
		// Register-Schreibschutz setzen
		EDIS;
}





