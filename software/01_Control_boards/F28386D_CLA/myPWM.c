//=================================================================================================
/// @file       myPWM.c
///
/// @brief      Datei enth�lt Variablen und Funktionen um die ePWM-Module eines TMS320F280049C
///							zu konfigurieren. Die ePWM1-Modul wird so initialisiert, dass am ePWM1A-Pin ein
///							Rechtecksignal mit 10 KHz und einem variablen Tastverh�ltnis (Aufl�sung: 500)
///							ausgegeben wird. ePWM8-Modul wird so konfiguriert, dass es alle 10 ms eine ADC-
///							Messung triggert.
///
/// @version    V1.2
///
/// @date       13.09.2022
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
//=== Function: PwmInitPwm1 =======================================================================
///
/// @brief  Funktion initialisiert das ePWM1-Modul mit 10 kHz Schaltfrequenz und einer
///					Aufl�sung von 500. Zus�tzlich wird GPIO 0 zur Augabe dieses Signals konfiguriert.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void PwmInitPwm1(void)
{
    // Register-Schreibschutz aufheben
    EALLOW;

    // Synchronisierungstakt w�hrend der Konfiguration ausschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    // Takt f�r das PWM1-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    __asm(" RPT #4 || NOP");
    // Takt-Teilerauf 20 setzen
    // TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)
    // EPWMCLK = SYSCLK / 2 = 100 MHz
    // (siehe "DeviceInit()" und S. 165 Reference Manual TMS320F2838x)
    EPwm1Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_1;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_10;
    // TBCTR nicht mit Wert aus dem Phasenregister laden
    EPwm1Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_DISABLE;
    // Betriebsart: hoch-runter z�hlen
    EPwm1Regs.TBCTL.bit.CTRMODE = PWM_TB_COUNT_UPDOWN;
    // Wert, welcher in TBPRD geschrieben wird, sofort �bernehmen
    EPwm1Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periode f�r 10 kHz Schaltfrequenz setzen
    EPwm1Regs.TBPRD = 500;
    // Compare-Register erst beschreiben, wenn der Z�hler den Wert 0 erreicht
		EPwm1Regs.CMPCTL.bit.SHDWAMODE = PWM_CC_SHADOW;
		EPwm1Regs.CMPCTL.bit.LOADAMODE = PWM_CC_SHDW_CTR_ZERO;
		// Tastverh�ltnis auf 0 setzen
		EPwm1Regs.CMPA.bit.CMPA = 0;
		// PWM1A-Pin auf high ziehen, wenn TBCTR (Timer-Z�hlwert) beim hochz�hlen den Wert CMPA erreicht
		EPwm1Regs.AQCTLA.bit.CAU = PWM_AQ_SET;
		// PWM1A-Pin auf low ziehen, wenn TBCTR (Timer-Z�hlwert) beim runterz�hlen den Wert CMPA erreicht
		EPwm1Regs.AQCTLA.bit.CAD = PWM_AQ_CLEAR;
		// Totzeit-Modul umgehen
		EPwm1Regs.DBCTL.bit.OUT_MODE = PWM_DB_BOTH_BYPASSED;
		// Timer auf 0 setzen
		EPwm1Regs.TBCTR = 0;
    // Synchronisierungstakt einschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;

    // Konfiguraionssperre f�r GPIO 0 aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO0 = 0;
    // GPIO auf PWM-Funktionalit�t setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = (0x01 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO0  = (0x01 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;

		// Register-Schreibschutz setzen
		EDIS;
}


//=== Function: PwmInitPwm8 =======================================================================
///
/// @brief  Funktion initialisiert das ePWM8-Modul um alle 10 ms einen
///					SOCA-Trigger auszul�sen um eine ADC-messung zu starten
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
		// Taktteiler: 160
    // TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)
    // EPWMCLK = SYSCLK / 2 = 100 MHz
    // (siehe "DeviceInit()" und S. 165 Reference Manual TMS320F2838x)
    EPwm8Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_16;
		EPwm8Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_10;
		// TBCTR nicht mit Wert aus dem Phasenregister laden
    EPwm8Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_DISABLE;
    // Daten, welche in das TBPRD Register (Periodendauer) geschrieben
    //  werden, direkt laden (ohne Umweg �ber das Shadow-Register)
    EPwm8Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periodendauer auf 10 ms setzen
    // Periodendauer = EPWMCLK / (HSPCLKDIV * CLKDIV * (TBPRD+1))
    EPwm8Regs.TBPRD = 6249;
		// Z�hler auf 0 setzen
		EPwm8Regs.TBCTR = 0;
		// Konfiguration des PWM-Moduls zum Triggern einer ADC-Messung:
		// SOCA-Trigger einschalten
    // 0: SOCA-Trigger ausgeschaltet
    // 1: SOCA-Trigger eingeschaltet
		EPwm8Regs.ETSEL.bit.SOCAEN = PWM_ET_SOC_ENABLE;
		// SOCA-Trigger erzeugen, wenn der PWM-Z�hler (TBCTR) den Wert 0 erreicht
		// 0: Trigger ausgeschaltet
    // 1: Trigger wenn TBCTR = 0
    // 2: Trigger wenn TBCTR = TBPRD
    // 3: Trigger wenn TBCTR = TBPRD oder TBCTR = 0
    // 4: Trigger wenn TBCTR = CMPA (SOCASELCMP = 0) bzw. TBCTR = CMPC (SOCASELCMP = 1) beim Hochz�hlen
    // 5: Trigger wenn TBCTR = CMPA (SOCASELCMP = 0) bzw. TBCTR = CMPC (SOCASELCMP = 1) beim Runterz�hlen
    // 6: Trigger wenn TBCTR = CMPB (SOCASELCMP = 0) bzw. TBCTR = CMPD (SOCASELCMP = 1) beim Hochz�hlen
    // 7: Trigger wenn TBCTR = CMPB (SOCASELCMP = 0) bzw. TBCTR = CMPD (SOCASELCMP = 1) beim Runterz�hlen
		EPwm8Regs.ETSEL.bit.SOCASEL = PWM_ET_CTR_ZERO;
		// SOCA-Trigger bei jedem Event (siehe ETSEL.SOCASEL) erzeugen
		// 0: Event-Trigger ausgeschaltet
		// 1: SOCA-Trigger beim 1. Auftreten des Events ausl�sen
		// 2: SOCA-Trigger beim 2. Auftreten des Events ausl�sen
		// n: SOCA-Trigger beim n. Auftreten des Events ausl�sen
		EPwm8Regs.ETPS.bit.SOCAPRD = PWM_ET_1ST;
    // Synchronisierungstakt wieder einschalten
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;

		// Register-Schreibschutz setzen
		EDIS;
}





