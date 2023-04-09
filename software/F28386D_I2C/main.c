//=================================================================================================
/// @file			main.c
///
/// @brief		Enthält das Hauptprogramm zur Demonstration des Moduls "myI2C.c". Die Funktionen
///						dieses Moduls implementieren eine Interrupt-basierte Kommunikation über die I2C-A
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
#include "myI2C.h"


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
		// I2C initialisieren mit 400 kHz SCL-Takt
		I2cInitA(I2C_CLOCK_400_KHZ);

		// Grundsätzlicher Ablauf einer I2C-Kommunikation:
		//
		// 1) Puffer "i2cBufferWrite[]" und "i2cBufferRead[]" initialisieren
		// 2) Zu sendene Daten in "i2cBufferWrite[]" schreiben, falls geschrieben werden soll
		// 3) Funktion "I2cWrite()", "I2cRead()" oder "I2CWriteRead()" aufrufen
		// 4) Rückgabewert prüfen, ob die Kommunikation gestartet wurde
		// 5) Warten bis die Kommunikation abgeschlossen ist ("I2cGetStatus()" abfragen")
		// 6) Empfangene Daten aus "i2cBufferRead[]" lesen, falls die Kommunikation erfolgreich war.
		//		Im Falle eines Fehler das I2C-Modul neu initialisieren
		// 7) I2C-Status auf "idle" setzen ("I2cSetStatusIdle()" aufrufen)


		// Um Daten über I2C an einen Slave zu senden,  müssen diese in den Sende-Puffer "i2cBufferWriteA[]"
		// geschrieben werden. Dabei müssen die Daten am Anfang des Puffers geschrieben werden (beginnend
		// vom Element 0 an). Es ist darauf zu achten, dass nicht mehr Daten geschrieben, als der Puffer
		// groß ist (I2C_SIZE_BUFFER_WRITE).
		i2cBufferWriteA[0] = 0xAA;
		i2cBufferWriteA[1] = 0xFF;
		i2cBufferWriteA[2] = 0x0F;
		i2cBufferWriteA[3] = 0xF0;


		// Vor jeder einer Kommunikation prüfen, ob ggf. noch eine vorherige Kommunikation aktiv ist
		if (I2cGetStatusA() == I2C_STATUS_IDLE)
		{
				// 3 Bytes an Slave-Adresse 0x48 senden
				if (!I2cWriteA(0x48, 2))
				{
						// Fehlerbehandlung:

				}
				/*
				// 3 Bytes vom Slave-Adresse 0x48 lesen
				if (!I2cReadA(0x48, 3))
				{
						// Fehlerbehandlung:

				}

				// 1 Byte an Slave-Adresse 0x48 senden und 2 Bytes empfangen
				if (!I2cWriteReadA(0x48, 1, 2))
				{
						// Fehlerbehandlung:

				}
				*/
		}
		// Warten bis die Kommunikation beendet ist
		while (I2cGetStatusA() == I2C_STATUS_IN_PROGRESS);

		// Vorangegangene Operation ist abgeschlossen
		if (I2cGetStatusA() == I2C_STATUS_FINISHED)
		{
				// Status-Flag auf "idle" setzen, um die Bereitschaft
				// der I2C-Schnittstelle zu signalisieren
				// (Rückgabewert wird hier nicht gebraucht)
				I2cSetStatusIdleA();
				// Ggf. vom Slave gelesene Daten auswerten:

		}
		// Vorangegangene Operation wurde aufgrund eines Fehlers abgebrochen
		else if (I2cGetStatusA() == I2C_STATUS_ERROR)
		{
				// Fehlerbehandlung: I2C-Modul neu initialisieren
				I2cInitA(I2C_CLOCK_100_KHZ);
		}

    // Register-Schreibschutz ausschalten
    EALLOW;

		// Dauerschleife Hauptprogramm
    while(1)
    {

    }
}

