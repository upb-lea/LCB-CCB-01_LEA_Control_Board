//=================================================================================================
/// @file			main.c
///
/// @brief		Enthält das Hauptprogramm zur Demonstration des Moduls "myDAC.c". Die Funktionen
///						dieses Moduls implementieren die Initialisierung und das Setzen der Ausgangsspannung
///						des Digital-Analog-Wandlers für den Mikrocontroller TMS320F2838x. Erklärungen zur
///						genauen Funktion sind im Modul "myDevice.c" zu finden. Die Ausgangsspannung wird
///						an Pin ADCINA0/DACOUTA ausgegeben.
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
#include "myDAC.h"


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
	  // DAC initialisieren
	  DacAInit();

    // Register-Schreibschutz ausschalten
    EALLOW;

		// Dauerschleife Hauptprogramm
    while(1)
    {
    		// DAC-Wert setzen
    		DacaRegs.DACVALS.bit.DACVALS = 0;
    }
}

