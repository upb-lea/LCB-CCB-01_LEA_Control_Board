//=================================================================================================
/// @file			main.c
///
/// @brief		Diese Datei enthält das Hauptprogramm für CPU 2 zur Demonstration der CPU-
///						übergreifenden Nutzung des Hardware-Monitors. CPU initialisiert die für das SPI-
///						Modul nötigen GPIOs und gibt die Kontrolle über das SPI-D an CPU 2. CPU 2 sendet
///						kontinuierlich Daten an den Hardware-Monitor. Diese Daten werden von CPU 1 mittels
///						eines von beiden CPUs genutzen RAM-Bereichs der CPU 2 zugänglich gemacht. So kann
///						CPU 1 ohne irgendeine Interaktion mit CPU 2 seine Aufgaben durchführen und rellevante
///						Daten werden durch CPU 2 an den Hardware-Monitor ausgegeben.
///
///					  https://software-dl.ti.com/C2000/docs/C2000_Multicore_Development_User_Guide/debug.html
///
/// @version	V1.0
///
/// @date			15.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "AD5664_cpu2.h"


// Dual-Core Debugging:
//
// Schritt 1: 	Target-Konfiguration von CPU1 oder CPU2 auswählen > Rechtsklick > Launch Selected
//							Configuration
//
// Schritt 2: 	Rechtsklick auf Debug-Probe CPU1 > Connect Target > Run > Load > Load Program… >
//							Browse project… > CPU 1-Projekt > FLASH bzw. RAM > .out-Datei auswählen > OK > OK
//
// Schritt 3: 	Rechtsklick auf Debug-Probe CPU2 > Connect Target > Run > Load > Load Program… >
//							Browse project… > CPU 2-Projekt > FLASH bzw. RAM > .out-Datei auswählen > OK > OK
//
// Schritt 4:		Debug-Probe CPU1 anklicken > Resume (Start) > Debug-Probe CPU2 anklicken > Resume
//							(Start)
//
// Schritt 5:		Zum Debuggen (Register/globale Variablen lesen/schreiben) muss die jeweilige Debug
//							-Probe (CPU 1 oder CPU 2) angeklickt werden. Die Register/Variablen der jeweils
//							anderen CPU sind während dessen nicht aktiv


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Daten im gemeinsamen RAM von CPU 1 und CPU 2.
// CPU 1 kann diese Daten lesen und schreiben,
// CPU 2 kann sie nur lesen
uint16_t fromCpu1[4];
#pragma DATA_SECTION(fromCpu1,"SHARERAMGS1");


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
		// Mikrocontroller initialisieren
		DeviceInit(DEVICE_DEFAULT);
		// SPI für die Kommunikation mit dem DAC mit 16 MHz SPI-Clock initialisieren
		AD5664Init(AD5664_SPI_CLOCK_16MHZ);

    // Register-Schreibschutz ausschalten
    EALLOW;


		// Einmal referenzieren damit die Variable im Debugger sichtbar ist
    // (der Schreibbefehl hat keine Auswirkung auf den Wert, da CPU 2
    // nur lesen kann)
		fromCpu1[0] = 0;


    while(1)
    {
    		// Kontinuierlich die Daten an den hardware-Monitor senden
        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_A,
												fromCpu1[0]);

        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_B,
												fromCpu1[1]);

        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_C,
												fromCpu1[2]);

        while (ad5664StatusFlag == AD5664_STATUS_IN_PROGRESS);
        AD566SetChannel(AD5664_CHANNEL_D,
												fromCpu1[3]);
    }
}



