//=================================================================================================
/// @file			main.c
///
/// @brief		Diese Datei enthält das Hauptprogramm für CPU 1 zur Demonstration der CPU-
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
#include "AD5664_cpu1.h"
#include "myDevice.h"


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
uint16_t toCpu2[4];
#pragma DATA_SECTION(toCpu2,"SHARERAMGS1");


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
		// SPI für Kommunikation mit DAC initialisieren
		AD5664Init();

    // Register-Schreibschutz ausschalten
    EALLOW;

    // Zugriffsberechtigung RAM-GSx:
    // 0: CPU1 hat Zugriff auf Speicher GSx
    // 1: CPU2 hat Zugriff auf Speicher GSx
    MemCfgRegs.GSxMSEL.bit.MSEL_GS1 = 0;


    // Einmal referenzieren damit die Variable im Debugger sichtbar ist
    toCpu2[0] = 0;


    while(1)
    {

    }
}


