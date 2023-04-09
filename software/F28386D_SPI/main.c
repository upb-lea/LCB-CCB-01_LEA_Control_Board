//=================================================================================================
/// @file			main.c
///
/// @brief		Enthält das Hauptprogramm zur Demonstration des Moduls "mySPI.c". Die Funktionen
///						dieses Moduls implementieren eine Interrupt-basierte Kommunikation über die SPI-A
///						Schnittstelle für den Mikrocontroller TMS320F2838x. Erklärungen zur genauen
///						Funktion sind im Modul zu finden.
///
/// @version	V1.2
///
/// @date			12.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "mySPI.h"


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
    // SPI als Master mit 1 MHz CLK-Takt initialisieren
    SpiInitA(SPI_CLOCK_1_MHZ);


    // Grundsätzlicher Ablauf einer SPI-Kommunikation:
    //
    // 1) Puffer "spiBufferRx[]" und "spiBufferTx[]" initialisieren
    // 2) Zu sendene Daten in "spiBufferTx[]" schreiben, falls geschrieben werden soll
    // 3) Funktion "SpiWrite()", "SpiRead()" oder "SpiWriteRead()" aufrufen
    // 4) Rückgabewert prüfen, ob die Kommunikation gestartet wurde
    // 5) Warten bis die Kommunikation abgeschlossen ist ("SpiGetStatus()" abfragen")
    // 6) Slave abwählen durch Aufruf von "SpiDisableSlave()"
    // 7) Empfangene Daten aus "spiBufferRx[]" lesen
    // 8) SPI-Status auf "idle" setzen ("SpiSetStatusIdle()" aufrufen)


    // Um Daten über SPI an einen Slave zu senden,  müssen diese in den Sende-Puffer "spiBufferTxA[]"
    // geschrieben werden. Dabei müssen die Daten am Anfang des Puffers geschrieben werden (beginnend
    // vom Element 0 an). Es ist darauf zu achten, dass nicht mehr Daten geschrieben, als der Puffer
    // groß ist (SPI_SIZE_BUFFER_TX).
		spiBufferTxA[0] = 1;
		spiBufferTxA[1] = 2;
		spiBufferTxA[2] = 3;
		spiBufferTxA[3] = 4;
		spiBufferTxA[4] = 5;

		// Vor jeder einer Kommunikation prüfen, ob ggf. noch eine vorherige Kommunikation aktiv ist
		if (SpiGetStatusA() == SPI_STATUS_IDLE)
		{
				// 3 Bytes an Slave 1 senden (nur senden, keine Nutzdaten in "spiBufferRxA[]" kopieren)
				if (!SpiWriteA(SPI_SELECT_SLAVE1, 3))
				{
						// Fehlerbehandlung:
						// ...
				}
				/*
				// 3 Bytes vom Slave 1 lesen (nur lesen, keine Nutzdaten aus "spiBufferTxA[]" senden)
				if (!SpiReadA(SPI_SELECT_SLAVE1, 3))
				{
						// Fehlerbehandlung:
						// ...
				}

				// 3 Bytes an Slave 1 senden und empfangen
				if (!SpiWriteReadA(SPI_SELECT_SLAVE1, 3))
				{
						// Fehlerbehandlung:
						// ...
				}
				*/
		}

		// Warten, bis die Kommunikation beendet ist
		while ((SpiGetStatusA() == SPI_STATUS_TX_IN_PROGRESS)
						|| (SpiGetStatusA() == SPI_STATUS_RX_IN_PROGRESS)
						|| (SpiGetStatusA() == SPI_STATUS_TX_RX_IN_PROGRESS));

		// Schreib-Operation ist abgeschlossen (Simplex-Betrieb)
		if (SpiGetStatusA() == SPI_STATUS_TX_FINISHED)
		{
				// Nach jeder abgeschlossenen Kommunikation muss der Slave
				// abgewählt werden. Aus Zeitgründen geschieht das nicht
				// in der SPI-ISR
				SpiDisableSlaveA();
				// Status-Flag auf "idle" setzen, um die Bereitschaft
				// der SPI-Schnittstelle zu signalisieren
				// (Rückgabewert wird hier nicht gebraucht)
				SpiSetStatusIdleA();
		}
		/*
		// Lese-Operation ist abgeschlossen (Simplex-Betrieb)
		if (SpiGetStatusA() == SPI_STATUS_RX_FINISHED)
		{
				// Nach jeder abgeschlossenen Kommunikation muss der Slave
				// abgewählt werden. Aus Zeitgründen geschieht das nicht
				// in der SPI-ISR
				SpiDisableSlaveA();
				// Status-Flag auf "idle" setzen, um die Bereitschaft
				// der SPI-Schnittstelle zu signalisieren
				// (Rückgabewert wird hier nicht gebraucht)
				SpiSetStatusIdleA();
				// Vom Slave gelesene Daten auswerten:
				// ...
		}

		// Schreib-Lese-Operation ist abgeschlossen (Duplex-Betrieb)
		if (SpiGetStatusA() == SPI_STATUS_TX_RX_FINISHED)
		{
				// Nach jeder abgeschlossenen Kommunikation muss der Slave
				// abgewählt werden. Aus Zeitgründen geschieht das nicht
				// in der SPI-ISR
				SpiDisableSlaveA();
				// Status-Flag auf "idle" setzen, um die Bereitschaft
				// der SPI-Schnittstelle zu signalisieren
				// (Rückgabewert wird hier nicht gebraucht)
				SpiSetStatusIdleA();
				// Vom Slave gelesene Daten auswerten:
				// ...
		}
		*/


    // Register-Schreibschutz ausschalten
    EALLOW;

		// Dauerschleife Hauptprogramm
    while(1)
    {

    }
}
