//=================================================================================================
/// @file			main.c
///
/// @brief		Enthält das Hauptprogramm zur Demonstration des Moduls "tripzone.c". Die Funktionen
///						dieses Moduls implementieren eine Verkettung von einem ADC-Eingangspin über die
///						internen Komparatoren bis hin zu den Tripzone-Modulen der ePWM1-Einheit um die
///						PWM-Ausgangspins abhänging von der Spannung an den ADC-Pins zu übersteuern. Der
///						Code ist für den Mikrocontroller TMS320F2838x. Erklärungen zur genauen Funktion
///						sind im Modul zu finden.
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
#include "myTripzone.h"
#include "myADC.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Messwert vom ADC A an Pin A2
uint16_t ADCINA2 = 0;


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
		// ADC initialisieren, um den Messwert und damit die Spannung an Pin A2
		// im Debugger anzeigen zu können. Dient zur Kontrolle der Tripzone-Funktion
		AdcAInit(ADC_RESOLUTION_12_BIT,
						 ADC_SINGLE_ENDED_MODE);
		// Tripzone initialisieren
		TripzoneInitCmpss1();

    // Register-Schreibschutz ausschalten
    EALLOW;

		// Dauerschleife Hauptprogramm
    while(1)
    {
				// Beispiel für eine manuell getriggerte ADC-Messung:
				// Dazu muss zuerst ADCSOC0CTL.TRIGSEL = 0 gesetzt werden
				// ADC-Messung triggern
				AdcaRegs.ADCSOCFRC1.bit.SOC0 = 1;
				// Warten bis die Wandlung abgeschlossen ist
				while (AdcaRegs.ADCCTL1.bit.ADCBSY);
				// Messwert auslesen
				ADCINA2 = AdcaResultRegs.ADCRESULT0;


				// Eingangssignal liegt oberhalb vom Grenzwert (DACHVALS.DACVAL)
				if (EPwm1Regs.TZFLG.bit.DCAEVT1)
				{
						// DCAEVT1-Event Tripzone-Flag löschen. Falls die Bedingung für das DCAEVT1-Event
						// erfüllt sind (Komparator-Ausgang = high), wird das Flag sofort wieder gesetzt
						// (siehe 5. Absatz von oben S. 2908 Reference Manual TMS320F2838x)
						EPwm1Regs.TZCLR.bit.DCAEVT1 = 1;
				}
				// Eingangssignal liegt unterhalb vom Grenzwert (DACLVALS.DACVAL)
				if (EPwm1Regs.TZFLG.bit.DCBEVT1)
				{
						// DCBEVT1-Event Tripzone-Flag löschen. Falls die Bedingung für das DCEVT1-Event
						// erfüllt sind (Komparator-Ausgang = low), wird das Flag sofort wieder gesetzt
						// (siehe 5. Absatz von oben S. 2908 Reference Manual TMS320F2838x)
						EPwm1Regs.TZCLR.bit.DCBEVT1 = 1;
				}
				// Ein One-Shot Event ist aufgetreten (DCAEVT1- oder DCBEVT1-Event)
				if (EPwm1Regs.TZFLG.bit.OST)
				{
						// Flag löschen
						EPwm1Regs.TZCLR.bit.OST = 1;
				}
    }
}

