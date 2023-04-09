//=================================================================================================
/// @file			main.c
///
/// @brief		Enthält das Hauptprogramm zur Demonstration des Moduls "myADC.c". Die Funktionen
///						dieses Moduls implementieren eine Interrupt-basierte und PWM-getriggerte Steuerung
///						des Analog-Digital-Wandlers für den Mikrocontroller TMS320F2838x. Erklärungen zur
///						genauen Funktion sind im Modul zu finden.
///
/// @version	V1.1
///
/// @date			06.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myADC.h"
#include "myPWM.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------


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
		// ADC initialisieren (Modul A)
		AdcAInit(ADC_RESOLUTION_12_BIT,
						 ADC_SINGLE_ENDED_MODE);
		// ePWM8-Modul initialisieren (zur PWM-getriggerten ADC-Messung)
		PwmInitPwm8();

		// GPIO 5 (LED D1002 auf dem ControlBoard) als Ausgang
		// konfigurieren zur Visualisierung des ADC-Triggers
		EALLOW;
		// Konfigurationssperre aufheben
		GpioCtrlRegs.GPALOCK.bit.GPIO5 = 0;
		// Auf GPIO-Funktionalität setzen
		GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = (0 >> 2);
		GpioCtrlRegs.GPAMUX1.bit.GPIO5  = (0 & 0x03);
		// Pull-Up-Widerstand deaktivieren
		GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;
		// Auf High-Pegel setzen
		GpioDataRegs.GPASET.bit.GPIO5 = 1;
		// Als Ausgang setzen
		GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;


		// Dauerschleife Hauptprogramm
		while(1)
		{
				/*
				// Beispiel für eine manuell getriggerte ADC-Messung:
				// Dazu muss zuerst ADCSOC0CTL.TRIGSEL = 0 gesetzt werden
				// ADC-Messung triggern
				AdcaRegs.ADCSOCFRC1.bit.SOC0 = 1;
				// Warten bis die Wandlung abgeschlossen ist
				while (AdcaRegs.ADCCTL1.bit.ADCBSY);
				// Messwert auslesen
				ADCIN0 = AdcaResultRegs.ADCRESULT0;
				*/
		}
}

