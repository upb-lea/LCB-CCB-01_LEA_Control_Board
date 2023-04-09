//=================================================================================================
/// @file       myDAC.c
///
/// @brief      Datei enth�lt Variablen und Funktionen um den internen Digital-Analog-Wandler
///							des TMS320F2838x zu nutzen. Der Code spricht das DAC-A Modul an. Die Nutzung
///							der Module B und C ist analog zu der des Moduls A. Die Referenzspannung ist
///							VREFHI und die DAC-Spannung wird an Pin ADCINA0/DACOUTA ausgegeben.
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
#include "myDAC.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: DacAInit ==========================================================================
///
/// @brief	Funktion initialisiert den DAC-A Modul
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void DacAInit(void)
{
		// Register-Schreibschutz aufheben
		EALLOW;

    // Takt f�r das DAC-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 283 Reference Manual TMS320F2838x)
		CpuSysRegs.PCLKCR16.bit.DAC_A = 1;
    __asm(" RPT #4 || NOP");
    // VREFHI als Referenzspannung verwenden
    // 0: VDAC (ADCINB0) zu VSS als Referenz
    // 1: VREFHI zu VSS als Referenz
    DacaRegs.DACCTL.bit.DACREFSEL = DAC_REF_VREFHI;
    // DAC-Wert beim n�chsten SYSCLK aus dem Schattenregister �bernehmen
    // 0: DAC-Wert �ber SYSCLK synchronisieren
    // 1: DAC-Wert �ber ePWM-Modul synchronisieren
    DacaRegs.DACCTL.bit.LOADMODE = DAC_SYNC_SYSCLK;
    // ePWM1 l�d den Wert vom DACVALS- in das DACVALA-Register
    // (nur aktiv falls LOADMODE = 1)
    // 0: EPWM1SYNCPER
    // 1: EPWM2SYNCPER
    // ...
    // 15: EPWM16SYNCPER
    DacaRegs.DACCTL.bit.SYNCSEL = DAC_EPWM1SYNCPER;
    /*
    // Initialisierung f�r das ePWM1-Modul zur DAC-Synchronisierung:
    // Takt f�r das PWM1-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    __asm(" RPT #4 || NOP");
    // Erweiterte Enstellung f�r den DAC- und CMPSS-Synchronisationsimpuls ausschalten
    // 0: Erweiterte Einstellung deaktiviert, Einstellung �ber PWMSYNCSEL
    // 4: TBCTR = CMPC (beim hoch z�hlen)
    // 5: TBCTR = CMPC (beim herunter z�hlen)
    // 6: TBCTR = CMPD (beim hoch z�hlen)
    // 7: TBCTR = CMPD (beim herunter z�hlen)
    EPwm1Regs.HRPCTL.bit.PWMSYNCSELX = 0;
    // Synchronisationsimpuls f�r DAC und CMPSS bei TBCTR = 0 senden
    // 0: Synchronisationsimpuls bei TBCTR = PRD
    // 1: Synchronisationsimpuls bei TBCTR = 0
    EPwm1Regs.HRPCTL.bit.PWMSYNCSEL = 1
		*/
    // Ausgang des DAC einschalten.
    // Das Ausgangssignal des DAC wird an Pin DACOUTA (ADCINA0) ausgegeben
    // 0: Ausgang ausgeschaltet
    // 1: Ausgang eingeschaltet
    DacaRegs.DACOUTEN.bit.DACOUTEN = DAC_ENABLE_OUTPUT;
    // Erst 500 �s nach Einschalten des ADC kann
    // eine korrekte Messung durchgef�hrt werden
		// (siehe "Power Up Time" S. 159 Datasheet TMS320F2838x)
    DELAY_US(500);

		// Register-Schreibschutz setzen
		EDIS;
}
