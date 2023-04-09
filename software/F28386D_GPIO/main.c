//=================================================================================================
/// @file			main.c
///
/// @brief		Enthält das Hauptprogramm zur Demonstration der GPIO-Funktionen für den Mikro-
///						controller TMS320F2838x. GPIO 3 wird als Ausgang konfiguriert und wechselt alle
/// 					500 ms den Pegel (low/high). GPIO 80 wird als Eingang mit aktiviertem Pull-Up
///						Widerstand konfiguriert und steuert kopiert seinen Zustand auf GPIO 3, falls das
///						zyklische Pegelwechseln auskommentiert wird (in main()). GPIO 90 ist ebenfalls als
///						Eingang mit aktiviertem Pull-Up Widerstand konfiguriert und löst bei einer fallenden
///						Flanke einen Interrupt aus.
///
/// @version	V1.1
///
/// @date			02.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myGPIO.h"


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
    // GPIOs initialisieren
    GpioInit();

    // Register-Schreibschutz ausschalten
    EALLOW;

		// Dauerschleife Hauptprogramm
    while(1)
    {

    		/*
    		// LED D1003 auf der ControlCard abhängig vom Pegel an GPIO 80 ein- bzw. ausschalten
    		if (GpioDataRegs.GPCDAT.bit.GPIO80 == 1)
    		{
    				GpioDataRegs.GPASET.bit.GPIO3 = 1;
    		}
    		else
    		{
    				GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
    		}
    		*/

    		// LED D1002 auf der ControlCard blinken
    		GpioDataRegs.GPATOGGLE.bit.GPIO5 = 1;
    		DELAY_US(500000);

    }
}

