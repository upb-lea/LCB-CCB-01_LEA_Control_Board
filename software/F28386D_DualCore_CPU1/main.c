//=================================================================================================
/// @file			main.c
///
/// @brief		Diese Datei enthält das Hauptprogramm als Basis zur Nutzung der CPU1 des
///						Mikrocontrollers TMS320F2838x. Zusätzlich ist eine einfache Kommunikation
///						zwischen CPU 1 und CPU 2 implementiert. Das Projekt kann als Grundlage für
///						neue Programme verwendet werden.
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
#include "myGPIO.h"
#include "myIPC.h"


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
		// Mikrocontroller initialisieren (Watchdog, Systemtakt, Speicher, Interrupts, CPU2 booten)
		DeviceInit(DEVICE_CLKSRC_EXTOSC_SE_25MHZ);
		// GPIOs initialisieren
		GpioInit();
		// Inter-Prozessor-Kommunikation initialisieren
		IpcInit();


    // Register-Schreibschutz ausschalten
    EALLOW;

    // Daten an CPU 2 senden
    IpcSendDataToCpu2(123);


    while(1)
    {
    		// LED D1002 auf dem ControlBoard blinken
    		GpioDataRegs.GPATOGGLE.bit.GPIO5 = 1;
    		DELAY_US(500000);
    }
}


