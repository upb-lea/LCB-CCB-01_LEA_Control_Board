//=================================================================================================
/// @file       myPWM.c
///
/// @brief      Datei enthält Variablen und Funktionen um die Funktion der ePWM-Module zu
///							demonstrieren. Das ePWM8-Modul wird so initialisiert, dass alle 5 ms ein
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
/// @brief  Funktion initialisiert das ePWM8-Modul um alle 5 ms einen Interrupt auszulösen
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
		// Taktteiler: 40
    // TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)
    // EPWMCLK = SYSCLK / 2 = 100 MHz
    // (siehe "DeviceInit()" und S. 165 Reference Manual TMS320F2838x)
    EPwm8Regs.TBCTL.bit.CLKDIV    = PWM_CLK_DIV_4;
		EPwm8Regs.TBCTL.bit.HSPCLKDIV = PWM_HSPCLKDIV_10;
		// TBCTR nicht mit Wert aus dem Phasenregister laden
    EPwm8Regs.TBCTL.bit.PHSEN = PWM_TB_PHSEN_DISABLE;
    // Daten, welche in das TBPRD Register (Periodendauer) geschrieben
    //  werden, direkt laden (ohne Umweg über das Shadow-Register)
    EPwm8Regs.TBCTL.bit.PRDLD = PWM_TB_IMMEDIATE;
    // Periodendauer auf 5 ms setzen
    // Periodendauer = EPWMCLK / (HSPCLKDIV * CLKDIV * (TBPRD+1))
    EPwm8Regs.TBPRD = 12500;
		// Zähler auf 0 setzen
		EPwm8Regs.TBCTR = 0;
		// PWM8-Interrupt einschalten
		EPwm8Regs.ETSEL.bit.INTEN = 1;
    // Interrupt auslösen, wenn der Timer den Endwert (TBPRD) erreicht hat
		EPwm8Regs.ETSEL.bit.INTSEL = PWM_ET_CTR_PRD;
    // ISR aufrufen, sobald 1 Interrupt ausgelöst wurde
		EPwm8Regs.ETPS.bit.INTPRD = PWM_ET_1ST;

    // Interrupt-Service-Routinen für den ePWM8-Interrupt an die
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


//=== Function: Epwm8ISR ==========================================================================
///
/// @brief  ISR wird aufgerufen, sobald der Timer 8 einen Überlauf hat. Wird als Zeitgeber für
///					den Aufruf der Funktion "UartGetStatusRxA()" verwendet. Diese Funktion überwacht den
///         Empfang von UART-Daten.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void Pwm8ISR(void)
{
		// Flag für den Aufruf der Funktion "UartGetStatusRx()" setzen
		uartFlagCheckRxA = true;
		// Timeout-Zähler herunterzählen
		if (uartRxTimeoutA > 0)
		{
				uartRxTimeoutA--;
		}

    // Interrupt-Flag im ePWM8-Modul löschen
		EPwm8Regs.ETCLR.bit.INT = 1;
    // Interrupt-Flag der Gruppe 3 löschen (da gehört der ePMW8-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK3 = 1;
}



