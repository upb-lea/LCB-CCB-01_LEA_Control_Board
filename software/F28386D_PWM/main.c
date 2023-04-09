//=================================================================================================
/// @file			main.c
///
/// @brief		Enth�lt das Hauptprogramm zur Demonstration des Moduls "myPWM.c". Die Funktionen
///						dieses Moduls implementieren eine Ansteuerung f�r einen 3-phasigen Wechselrichter
///						sowie einen Zeitgeber f�r z.B. periodisch zu bearbeitende Aufgaben f�r den
///						Mikrocontroller TMS320F2838x. Erkl�rungen zur genauen Funktion sind im Modul
///						zu finden.
///
/// @version	V1.1
///
/// @date			05.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
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
	  // ePWM1, ePWM2 und ePWM3-Modul zur Ansteuerung eines
	  // 3-phasigen Wechselrichters initialisieren
	  PwmInitPwm123();
	  // ePWM8-Modul als 100 ms-Zeitgeber initialisieren
	  PwmInitPwm8();

    // Register-Schreibschutz ausschalten
    EALLOW;

		// Dauerschleife Hauptprogramm
    while(1)
    {

    }
}

