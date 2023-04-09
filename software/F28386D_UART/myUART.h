//=================================================================================================
/// @file       uart.h
///
/// @brief      Datei enthält Variablen und Funktionen um die UART-Schnittstelle (SCI) eines
///							TMS320F283x zu benutzen. Die Kommunikation ist Interrupt-basiert. Zum Senden wird
////						die Funktion "UartTransmit()" aufgerufen und die zu sendene Anzahl an Bytes als
///							Parameter übergeben. Die zu sendenen Daten werden zuvor in den Puffer
///							"uartBufferTx[]" geschrieben. Der Empfangsvorgang wird durch Aufruf der Funktion
///							"UartReceive()" freigegeben. Da UART asynchron ist, kann der Empfangsvorgang nur
///							freigegeben, aber nicht aktiv forciert werden. Nach Aufruf der Funktion
///							"UartReceive()", sollte in regelmäßigen Abständen die Funktion "UartGetStatusRx()"
///							aufgerufen werden um den korekten/vollständigen Empfang eines Datenpakets zu
///							prüfen. Näheres ist der Beschreibung der Funktion "UartGetStatusRx()" zu entnehmen.
///							Es wird das SCI-A Modul verwendet. Die Module B, C und D können analog zu den hier
///							gezeigten Funktionen verwendet werden.
///
/// @version    V1.2
///
/// @date       09.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
#ifndef MYUART_H_
#define MYUART_H_


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Größe der Software-Puffer
#define UART_SIZE_BUFFER_RX		                  30
#define UART_SIZE_BUFFER_TX		                  30
// Zustände der UART-Kommunikation (uartRxStatusFlag und uartTxStatusFlag)
#define UART_STATUS_IDLE												0
#define UART_STATUS_IN_PROGRESS									1
#define UART_STATUS_RX_TIMEOUT									2
#define UART_STATUS_FINISHED										3
// Werte für den Timeout-Zähler beim Empfangen von Daten. Wird innerhalb des
// gewählten Zeitfensters (übergeben beim Aufruf der Funktion "UartReceive()")
// kein vollständiges Datenpaket empfangen, so wird der Empfangsvorgang abgerochen
#define UART_NO_TIMEOUT													-1
#define UART_10_MS_TIMEOUT											2
#define UART_20_MS_TIMEOUT											4
#define UART_50_MS_TIMEOUT											10
#define UART_100_MS_TIMEOUT											20
#define UART_200_MS_TIMEOUT											40
#define UART_500_MS_TIMEOUT											100
#define UART_1_S_TIMEOUT												200
#define UART_2_S_TIMEOUT												400
#define UART_5_S_TIMEOUT												1000
#define UART_10_S_TIMEOUT												2000
#define UART_20_S_TIMEOUT												4000
#define UART_1_M_TIMEOUT												12000
#define UART_2_M_TIMEOUT												24000
#define UART_5_M_TIMEOUT												60000
// Baud-Raten
#define UART_BAUD_2400													2400
#define UART_BAUD_4800													4800
#define UART_BAUD_9600													9600
#define UART_BAUD_19200			  									19200
#define UART_BAUD_38400 												38400
#define UART_BAUD_115200												115200
#define UART_BAUD_230400												230400
#define UART_BAUD_460800												460800
// Wortlänge
#define UART_DATA_1_BIT													0
#define UART_DATA_2_BIT													1
#define UART_DATA_3_BIT													2
#define UART_DATA_4_BIT													3
#define UART_DATA_5_BIT													4
#define UART_DATA_6_BIT													5
#define UART_DATA_7_BIT													6
#define UART_DATA_8_BIT													7
// Länge Stopbit
#define UART_STOP_1_BIT													0
#define UART_STOP_2_BIT													1
// Parität
#define UART_PARITY_NONE												0
#define UART_PARITY_EVEN  											1
#define UART_PARITY_ODD													2


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Software-Puffer für die UART-Kommunikation (SCI-A)
extern uint16_t uartBufferRxA[UART_SIZE_BUFFER_RX];
extern uint16_t uartBufferTxA[UART_SIZE_BUFFER_TX];
// Flag kann zum Aufruf der Funktion "UartGetStatusRxA()" genutzt werden
// und sollte dazu regelmäßig (z.B. alle 5 ms) in einer ISR gesetzt werden.
// Anschließend kann z.B. im Hauptprogramm bei gesetztem Flag die Funktion
// aufgerufen und das Flag gelöscht werden
extern bool uartFlagCheckRxA;
// Timeout-Zähler für den Empfang eines Datenpakets
extern int32_t uartRxTimeoutA;


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion initialisiert das UART-Modul (SCI-A)
// und die GPIOs für die Kommunikation über UART
extern void UartInitA(uint32_t baud,
										  uint32_t numberOfDataBits,
										  uint32_t numberOfStopBits,
										  uint32_t parity);
// Funktion initialisiert den UART Empfangs-Softwarepuffer zu 0
extern void UartInitBufferRxA(void);
// Funktion initialisiert den UART Sende-Softwarepuffer zu 0
extern void UartInitBufferTxA(void);
// Funktion prüft den Empfang eines Datenpakets über UART und
// gibt den aktuellen Status der Empfangs-Kommuniktaion zurück
extern uint16_t UartGetStatusRxA(void);
// Funktion gibt den aktuellen Status der Tx-UART-Kommunikation (senden) zurück
extern uint16_t UartGetStatusTxA(void);
// Funktion setzt das Status-Flag für den Empfangsvorgang auf "idle",
// falls die vorherige Kommunikation abgeschlossen ist
extern bool UartSetStatusIdleRxA(void);
// Funktion setzt das Status-Flag für den Sendevorgang auf "idle",
// falls die vorherige Kommunikation abgeschlossen ist
extern bool UartSetStatusIdleTxA(void);
// Funktion initialisiert Steuervariablen und den Rx-Interrupt, um die mit
// dem Parameter "numberOfBytesRxA" angegebene Anzahl an Bytes zu empfangen
extern bool UartReceiveA(uint16_t numberOfBytesRx,
												 int32_t timeOut);
// Funktion sendet über UART die mit dem Parameter "numberOfBytesTxA"
// angegebene Anzahl an Bytes aus dem Software-Puffer "uartBufferTxA[]"
extern bool UartTransmitA(uint16_t numberOfBytesTx);
// Interrupt-Service-Routine für die UART-Kommunikation (Aufruf, wenn ein Byte empfangen wurde)
__interrupt void UartRxISRA(void);
// Interrupt-Service-Routine für die UART-Kommunikation (Aufruf, nachdem ein Byte gesendet wurde)
__interrupt void UartTxISRA(void);


#endif

