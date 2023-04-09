//=================================================================================================
/// @file			main.c
///
/// @brief		Enthält das Hauptprogramm zur Demonstration des Moduls "myUART.c". Die Funktionen
///						dieses Moduls implementieren eine Interrupt-basierte Kommunikation über die UART-
///						Schnittstelle für den Mikrocontroller TMS320F2838x. Erklärungen zur genauen
///						Funktion sind im Modul zu finden.
///
///						UART über XDS100V2 freischalten:
///						- Template für Debugger herunterladen (http://processors.wiki.ti.com/index.php/XDS100
///							Version: Standalone XDS100v1)
///						- Template in FT_Prog öffnen
///						- Hardware Specific > Port B > Hardware > RS232 UART auswählen
///						- Hardware Specific > Port B > Driver > Virtual COM Port auswählen
///
///						LEA ControlBoard bekommt keinen COM-PORT zugewiesen:
///						Geräte-Manager > TI Debug Probes > Rechtsklick auf Auxiliary Port > Eigenschaften
///						> Erweitert > VCP laden aktivieren > Debugger vom PC trennen und neu verbinden
///
/// @version	V1.2
///
/// @date			09.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myUART.h"
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
// Steuert das Senden eines UART-Datenpakets
uint32_t uartTransmitPackageA = 0;


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
    // UART (SCI-A) initialisieren
    UartInitA(UART_BAUD_115200,
						  UART_DATA_8_BIT,
						  UART_STOP_1_BIT,
						  UART_PARITY_NONE);
		// Timer 8 als Zeitgeber initialisieren
    PwmInitPwm8();


    // Ablauf einer UART Empfangs-Kommunikation:
    //
    // 1) Mit "UartGetStatusRx()" sicherstellen, dass aktuell keine andere Kommunikation aktiv ist
    // 2) Puffer "uartBufferRx[]" initialisieren
    // 3) Funktion "UartReceive(n)" aufrufen, n = Anzhal der zu sendenen Bytes
    // 4) Regelmäßig die Funktion "UartGetStatusRx()" aufrufen um den Empfang zu überwachen
    // 5) Wenn die Funktion "UartGetStatusRx()" "UART_STATUS_FINISHED" zurückgibt, die Daten auswerten
    // 6) Kommunikations-Status auf "idle" setzen mit "UartSetStatusIdleRx()"
    //
    //
    // Ablauf einer UART Sende-Kommunikation:
    //
    // 1) Mit "UartGetStatusTx()" sicherstellen, dass aktuell keine andere Kommunikation aktiv ist
    // 2) Puffer "uartTxBuffer[]" initialisieren
    // 3) Zu sendene Daten in "uartTxBuffer" schreiben
    // 4) Funktion "UartTransmit(n)" aufrufen, n = Anzhal der zu sendenen Bytes
    // 5) Warten bis die Kommunikation abgeschlossen ist durch Abfrage von "UartGetStatusTx()"
    // 6) Kommunikations-Status auf "idle" setzen mit "UartSetStatusIdleTx()"


		// GPIO 5 (LED D1002 auf ControlBoard) als Ausgang
		// konfigurieren zur Visualisierung der UART-Kommunikation
		EALLOW;
		// Konfigurationssperre für GPIO 5 aufheben
		GpioCtrlRegs.GPALOCK.bit.GPIO5 = 0;
		// GPIO 5 auf GPIO-Funktionalität setzen
		GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = (0x00 >> 2);
		GpioCtrlRegs.GPAMUX1.bit.GPIO5  = (0x00 & 0x03);
		// Pull-Up-Widerstand von GPIO 5 deaktivieren
		GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;
		// GPIO 5 auf High-Pegel setzen
		GpioDataRegs.GPASET.bit.GPIO5 = 1;
		// GPIO 5 als Ausgang setzen
		GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;


		// Dauerschleife Hauptprogramm
    while(1)
    {
    		// Solange das Bit nicht gesetzt ist, sind noch Daten im
    		// Tx-Ausgangs-Schieberegister und/oder im Tx-Ausgangsregister.
    		// Kann z.B. zur Umschaltung eines RS485-Treibers genutzt werden
    		if (SciaRegs.SCICTL2.bit.TXEMPTY)
    		{
    				// RS485-Treiber auf "Empfang" umschalten
    				// ...
    		}


    		// SENDEN:
    		// Datenpaket nur senden, falls keine andere Sende-Kommunikation aktiv ist
    		if (   (UartGetStatusTxA() == UART_STATUS_IDLE)
    				&& (uartTransmitPackageA == 1))
    		{
            // Daten, welche versendet werden sollen, in den Sende-Puffer schreiben
    				uartBufferTxA[0] = 1;
            uartBufferTxA[1] = 2;
            uartBufferTxA[2] = 3;
            // Sendevorgang starten (3 Bytes senden)
        		if (!UartTransmitA(3))
        		{
        				// Fehlerbehandlung:
        				// ...
        		}
        		// Datenpaket nur einmal senden
        		uartTransmitPackageA = 0;
    		}
    		// Datenpaket wurde vollständig gesendet
    		else if (UartGetStatusTxA() == UART_STATUS_FINISHED)
    		{
    				// Sende-Kommunikation in den Zustand "idle" (bereit)
    				// versetzen (Rückgabewert wird hier nicht gebraucht)
    				UartSetStatusIdleTxA();
    		}


    		// EMPFANGEN:
    		// Alle 5 ms die Funktion "UartGetStatusRxA()" zur
    		// Überwachung des Empfangsvorgangs aufrufen
    		if (uartFlagCheckRxA)
    		{
    				// Status abfragen (darf nicht mehrmals in der if-Abfrage geschehen,
						// da zwischen zwei Abfragen mindestens 10/Baud Sekunden vergehen müssen)
						uint16_t state = UartGetStatusRxA();
    				// Es ist keine Empfangs-Kommunikation aktiv
    				if (state == UART_STATUS_IDLE)
						{
								// Den Empfang von 5 Datenbytes ohne Timeout initiieren
								if (!UartReceiveA(5, UART_NO_TIMEOUT))
								{
										// Fehlerbehandlung:
										// ...
								}
						}
    				// Es wurde ein vollständiges Datenpaket empfangen
    				else if (state == UART_STATUS_FINISHED)
    				{
    						// Empfangene Daten auswerten:
    		    		// LED D1002 auf dem ControlBoard umschalten um
    						// den Empfang eines Datenpakets zu signalisieren
    						GpioDataRegs.GPATOGGLE.bit.GPIO5 = 1;

    						// Empfangs-Kommunikation in den Zustand "idle" (bereit)
    						// versetzen (Rückgabewert wird hier nicht gebraucht)
    						UartSetStatusIdleRxA();
    				}
    				// Es ist ein Timeout aufgetreten (kein vollständiger
    				// Datenempfang innerhalb des definerten Zeitfensters)
    				else if (state == UART_STATUS_RX_TIMEOUT)
    				{
    						// Fehlerbehandlung:
    						// ...
    						// Empfangs-Kommunikation in den Zustand "idle" (bereit)
								// versetzen (Rückgabewert wird hier nicht gebraucht)
								UartSetStatusIdleRxA();
    				}

    				// SCI-Modul reseten, falls ein Rx-Fehler aufgetreten ist.
    				// Z.B. passiert dies, wenn der Rx-Interrupt ausgeschaltet
    				// ist und Daten empfangen werden. Dann wird der Rx-Puffer
    				// nicht ausgelesen und es kommt zu einem Überlauf
						if (SciaRegs.SCIRXST.bit.RXERROR == 1)
						{
								SciaRegs.SCICTL1.bit.SWRESET = 0;
								SciaRegs.SCICTL1.bit.SWRESET = 1;
						}

    				// Flag löschen, damit die Funktion "UartGetStatusRx()"
    				// erst in 5 ms wieder aufgerufen wird
    				uartFlagCheckRxA = false;
    		}

    }
}



