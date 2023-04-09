//=================================================================================================
/// @file			main.c
///
/// @brief		Diese Datei enthält das Hauptprogramm als Basis zur Nutzung des Mikrocontrollers
///						TMS320F2838x. Das Projekt kann als Grundlage für neue Programme verwendet werden.
///						Es kann direkt auf die Register des Mikrocontrollers zugegriffen werden.
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
#include "myDevice.h"
#include "AD5664.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
uint16_t dataDacA = 0;
uint16_t dataDacB = 0;
uint16_t dataDacC = 0;
uint16_t dataDacD = 0;


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
		// DAC mit 16 MHz SPI-Clock initialisieren
		AD5664Init(AD5664_SPI_CLOCK_16MHZ);


    // Register-Schreibschutz ausschalten
    EALLOW;


		// Dauerschleife Hauptprogramm
    while(1)
    {
        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_A,
												dataDacA);


        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_B,
												dataDacB);


        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_C,
												dataDacC);


        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_D,
												dataDacD);
    }
}

