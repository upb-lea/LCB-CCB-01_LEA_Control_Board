//=================================================================================================
/// @file       uart.c
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
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myUART.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Software-Puffer für die UART-Kommunikation
uint16_t uartBufferRxA[UART_SIZE_BUFFER_RX];
uint16_t uartBufferTxA[UART_SIZE_BUFFER_RX];
// Steuern das Kopieren in und aus den Software-Puffern während der UART-Kommunikation
uint16_t uartBufferIndexRxA;
uint16_t uartBufferIndexTxA;
uint16_t uartBufferIndexRxOldA;
uint16_t uartBytesToTransferRxA;
uint16_t uartBytesToTransferTxA;
// Flags speichern den aktuellen Zustand der UART-Kommunikation
uint16_t uartStatusFlagRxA;
uint16_t uartStatusFlagTxA;
// Flag kann zum Aufruf der Funktion "UartGetStatusRxA()" genutzt werden
// und sollte dazu regelmäßig (z.B. alle 5 ms) in einer ISR gesetzt werden.
// Anschließend kann z.B. im Hauptprogramm bei gesetztem Flag die Funktion
// aufgerufen und das Flag gelöscht werden
bool uartFlagCheckRxA;
// Timeout-Zähler für den Empfang eines Datenpakets
int32_t uartRxTimeoutA = UART_NO_TIMEOUT;


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: UartInitA =========================================================================
///
/// @brief  Funktion initialisiert GPIO 28 und 135 als UART-Pins und das
///         SCI-A Modul für den UART-Betrieb mit der gewünschten Baudrate.
///
/// @param  uint32_t baud, uint32_t numberOfDataBits, uint32_t numberOfStopBits, uint32_t parity
///
/// @return void
///
//=================================================================================================
void UartInitA(uint32_t baud,
							 uint32_t numberOfDataBits,
							 uint32_t numberOfStopBits,
							 uint32_t parity)
{
    // Register-Schreibschutz aufheben
    EALLOW;

    // Rx-Pin
    // GPIO-Sperre für GPIO 28 aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO28 = 0;
    // GPIO 28 auf UART-Funktion setzen (RxD)
    // Die Zahl in der obersten Zeile der Tabelle gibt den Wert für
    // GPAGMUX (MSB, 2 Bit) + GPAMUX (LSB, 2 Bit) als Dezimalzahl an.
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPAGMUX2.bit.GPIO28 = (1 >> 2);
    GpioCtrlRegs.GPAMUX2.bit.GPIO28  = (1 & 0x03);
    // GPIO 28 Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 1;
    // GPIO 28 Asynchroner Eingang (muss für UART gesetzt sein)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 0x03;

    // Tx-Pin:
    // GPIO-Sperre aufheben
    GpioCtrlRegs.GPELOCK.bit.GPIO135 = 0;
    // Auf UART-Funktion setzen (TxD)
    // Die Zahl in der obersten Zeile der Tabelle gibt den Wert für
    // GPAGMUX (MSB, 2 Bit) + GPAMUX (LSB, 2 Bit) als Dezimalzahl an.
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPEGMUX1.bit.GPIO135 = (6 >> 2);
    GpioCtrlRegs.GPEMUX1.bit.GPIO135  = (6 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPEPUD.bit.GPIO135 = 1;
    // Asynchroner Eingang (muss für UART gesetzt sein)
    GpioCtrlRegs.GPEQSEL1.bit.GPIO135 = 0x03;

    // Takt für das UART-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR7.bit.SCI_A = 1;
    __asm(" RPT #4 || NOP");
    // FIFO-Modus deaktivieren
    SciaRegs.SCIFFTX.bit.SCIFFENA = 0;
    // RxD und TxD während der Konfiguration ausschalten
    SciaRegs.SCICTL1.bit.RXENA = 1;
    SciaRegs.SCICTL1.bit.TXENA = 1;
    // Baudrate setzen
    // (Low-Speed CLK / (BAUD * SCICHAR)) - 1
    // Low-Speed CLK = 50 MHz (siehe "DeviceInit()")
    uint32_t divider = (50000000 / (baud * 8U)) - 1U;
    SciaRegs.SCIHBAUD.bit.BAUD = (divider & 0xFF00) >> 8;
    SciaRegs.SCILBAUD.bit.BAUD =  divider & 0x00FF;
    // Anzahl der Datenbits setzen
    SciaRegs.SCICCR.bit.SCICHAR = numberOfDataBits;
    // Anzahl der Stopbits setzen
    SciaRegs.SCICCR.bit.STOPBITS = numberOfStopBits;
    // Parität setzen
		switch (parity)
		{
				// Gerade Parität
				case UART_PARITY_EVEN:
						SciaRegs.SCICCR.bit.PARITYENA = 1;
						SciaRegs.SCICCR.bit.PARITY    = 1;
						break;
				// Ungerade Parität
				case UART_PARITY_ODD:
						SciaRegs.SCICCR.bit.PARITYENA = 1;
						SciaRegs.SCICCR.bit.PARITY    = 0;
						break;
				// Keine Parität
				default:
						SciaRegs.SCICCR.bit.PARITYENA = 0;
		}
    // RxD und TxD wieder einschalten
    SciaRegs.SCICTL1.bit.RXENA = 1;
    SciaRegs.SCICTL1.bit.TXENA = 1;
    // Reset deaktivieren
    SciaRegs.SCICTL1.bit.SWRESET = 1;

    // CPU-Interrupts während der Konfiguration global sperren
    DINT;
    // Interrupt-Service-Routinen für den RxD-Interrupt an die
    // entsprechende Stelle (SCIA_RX_INT) der PIE-Vector Table speichern
    PieVectTable.SCIA_RX_INT = &UartRxISRA;
    // SCIA_RX-Interrupt freischalten (Zeile 9, Spalte 1 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
    // Interrupt-Service-Routinen für den TxD-Interrupt an die
    // entsprechende Stelle (SCIA_TX_INT) der PIE-Vector Table speichern
    PieVectTable.SCIA_TX_INT = &UartTxISRA;
    // SCIA_TX-Interrupt freischalten (Zeile 9, Spalte 2 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
    // CPU-Interrupt 9 einschalten (Zeile 9 der Tabelle 3-2)
    IER |= M_INT9;
    // CPU-Interrupts nach Konfiguration global wieder freigeben
    EINT;

		// Register-Schreibschutz setzen
		EDIS;

    // Software-Puffer inititalisieren
    UartInitBufferRxA();
    UartInitBufferTxA();
    // Steuervariablen initialisieren
    uartBufferIndexRxA     = 0;
    uartBufferIndexTxA     = 0;
    uartBufferIndexRxOldA  = uartBufferIndexRxA;
    uartBytesToTransferRxA = 0;
		uartBytesToTransferTxA = 0;
		uartStatusFlagRxA      = UART_STATUS_IDLE;
		uartStatusFlagTxA      = UART_STATUS_IDLE;
		uartFlagCheckRxA       = false;
		uartRxTimeoutA         = UART_NO_TIMEOUT;
}


//=== Function: UartInitBufferRxA =================================================================
///
/// @brief  Funktion initialisiert alle Elemente des UART Software-Empfangspuffers zu 0.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void UartInitBufferRxA(void)
{
    for(uint16_t i=0; i<UART_SIZE_BUFFER_RX; i++)
    {
        uartBufferRxA[i] = 0;
    }
}


//=== Function: UartInitTxBufferA =================================================================
///
/// @brief  Funktion initialisiert alle Elemente des UART Software-Sendepuffers zu 0.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void UartInitBufferTxA(void)
{
    for(uint16_t i=0; i<UART_SIZE_BUFFER_TX; i++)
    {
        uartBufferTxA[i] = 0;
    }
}


//=== Function: UartGetStatusRxA ==================================================================
///
/// @brief	Funktion gibt den aktuellen Status der UART-Kommunikation (Empfangs-Prozess)
///					zurück und prüft den Empfang von Daten, d.h. es wird abhängig vom Wert der
///					Variable "uartBytesToTransferRxA" (wird über die Funktion "UartReceiveA()" gesetzt)
///					die Gültigkeit der empfangenen Daten gepüft (Länge des empfangenen Datenpakets).
///					Damit diese Prüfung funktioniert, sollte die Funktion min. alle 100 ms und max. alle
///					5 ms aufgerufen werden. Die max. Zeit zwischen den Aufrufen richtet sich danach, wie
///					oft ein Datenpaket zu erwarten ist, die min. Zeit danach, wie lange ein Byte zur
///					Übertragung benötigt (1/Baudrate). Die Kommunikation ist Interrupt-basiert. Die
///					Funktion gibt folgende Zustände der Kommunikation zurück:
///
///					- UART_STATUS_IDLE       : Es ist keine Empfangs-Kommunikation aktiv
///					- UART_STATUS_IN_PROGRESS: Es werden gerade Daten empfangen
///					- UART_STATUS_FINISHED   : Eine Empfangs-Kommunikation ist abgeschlossen
///
///					Zum starten einer Empfangs-Kommunikation muss die Funktionen "UartReceiveA()"
///					aufgerufen werden. Dabei wird nicht aktiv ein Empfang durcgeführt, da UART asynchron ist.
///					Siehe hierzu die Beschreibung der Funktion "UartReceiveA()".
///
/// @param	void
///
/// @return uint16_t uartStatusFlagRx
///
//=================================================================================================
extern uint16_t UartGetStatusRxA(void)
{
		// In der Zeit zum vorherigen Aufruf dieser Funktion wurden keine neuen
		// Daten empfangen -> Kommunikation beendet oder noch nicht gestartet.
		// Die if-Bedinung tritt entweder ein, wenn zwischen zwei Funktionsaufrufen
		// keine Daten empfangen wurden oder der Empfangspuffer voll ist. Letzteres
		// deutet auf einen zu klein gewählten Empfangspuffer hin
		if (uartBufferIndexRxA == uartBufferIndexRxOldA)
		{
				// Datenpaket ist vollständig/gültig
				if (uartBufferIndexRxA == uartBytesToTransferRxA)
				{
						// Interrupt ausschalten, damit bis zur Auswertung
						// der Daten keine weiteren Daten empfangen werden
						SciaRegs.SCICTL2.bit.RXBKINTENA = 0;
						// Timeout-Zähler initialisieren
						uartRxTimeoutA = UART_NO_TIMEOUT;
						// Status-Flag setzen
						if (uartStatusFlagRxA == UART_STATUS_IN_PROGRESS)
						{
								uartStatusFlagRxA = UART_STATUS_FINISHED;
						}
				}
				// Datenpaket ist unvollständig/ungültig
				else if (uartBufferIndexRxA > 0)
				{
						// Empfangs-Interrupt ausschalten
						SciaRegs.SCICTL2.bit.RXBKINTENA = 0;
						// Puffer und Empfangsvariablen initialisieren
						UartInitBufferRxA();
						uartBufferIndexRxA    = 0;
						uartBufferIndexRxOldA = 0;
						// Status auf "bereit" setzen
						uartStatusFlagRxA = UART_STATUS_IDLE;
						// Empfangs-Interrupt wieder einschalten
						SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
				}
		}
		// In der Zeit zum vorherigen Aufruf dieser Funktion wurden
		// weitere Daten empfangen -> Kommunikation noch aktiv
		else
		{
				// Alten und neuen Zähler synchronisieren
				uartBufferIndexRxOldA = uartBufferIndexRxA;
		}

		// Innerhalb des vorgegebenen Zeitfensters (wird bei Aufruf der Funktion
		// "UartReceiveA()" übergeben) wurde kein vollständiges Datenpaket empfangen
		if (uartRxTimeoutA == 0)
		{
				// Zähler initialisieren, damit der Empfangsvorgang
				// nur einmalig abgebochen wird
				uartRxTimeoutA = UART_NO_TIMEOUT;
				// Empfangsinterrupt ausschalten
				SciaRegs.SCICTL2.bit.RXBKINTENA = 0;
				// Staus auf "Timeout" setzen
				uartStatusFlagRxA = UART_STATUS_RX_TIMEOUT;
		}

		return uartStatusFlagRxA;
}


//=== Function: UartGetStatusTxA ==================================================================
///
/// @brief	Funktion gibt den aktuellen Status der Tx UART-Kommunikation (senden) zurück.
///					Die Kommunikation ist Interrupt-basiert und kann folgende Zustände annehmen:
///
///					- UART_STATUS_IDLE       : Es ist keine Sende-Kommunikation aktiv
///					- UART_STATUS_IN_PROGRESS: Eine Sende-Kommunikation wurde gestartet
///					- UART_STATUS_FINISHED   : Das letzte Byte wurde begonnen zu senden,
///																		 ist aber noch nicht koplett versendet
///
///					Zum starten einer Sende-Kommunikation muss die Funktionen "UartTransmitA()"
///					aufgerufen werden.
///
/// @param	void
///
/// @return uint16_t uartStatusFlagTx
///
//=================================================================================================
extern uint16_t UartGetStatusTxA(void)
{
		return uartStatusFlagTxA;
}


//=== Function: UartSetStatusIdleRxA ==============================================================
///
/// @brief	Funktion setzt das Rx Status-Flag auf "idle" und gibt "true" zurück, falls die vorherige
///					Kommunikation abgeschlossen ist. Ist noch eine Kommunikation aktiv, wird das Flag nicht
///					verändert und es wird "false" zurück gegeben.
///
/// @param	void
///
/// @return bool flagSetToIdle
///
//=================================================================================================
extern bool UartSetStatusIdleRxA(void)
{
		bool flagSetToIdle = false;
		// Staus-Flag nur auf "idle" setzen, falls eine
		// vorherige Kommunikation abgeschlossen ist
		if (uartStatusFlagRxA != UART_STATUS_IN_PROGRESS)
		{
				uartStatusFlagRxA = UART_STATUS_IDLE;
				flagSetToIdle = true;
		}
		return flagSetToIdle;
}


//=== Function: UartSetStatusIdleTxA ==============================================================
///
/// @brief	Funktion setzt das Tx Status-Flag auf "idle" und gibt "true" zurück, falls die vorherige
///					Kommunikation abgeschlossen ist. Ist noch eine Kommunikation aktiv, wird das Flag nicht
///					verändert und es wird "false" zurück gegeben.
///
/// @param	void
///
/// @return bool flagSetToIdle
///
//=================================================================================================
extern bool UartSetStatusIdleTxA(void)
{
		bool flagSetToIdle = false;
		// Staus-Flag nur auf "idle" setzen, falls eine
		// vorherige Kommunikation abgeschlossen ist
		if (uartStatusFlagTxA == UART_STATUS_FINISHED)
		{
				uartStatusFlagTxA = UART_STATUS_IDLE;
				flagSetToIdle = true;
		}
		return flagSetToIdle;
}


//=== Function: UartReceiveA ======================================================================
///
/// @brief  Funktion konfiguriert die Steuervariablen und den Empfangs-Interrupt so, dass Daten
///					über UART empfangen werden können. Der Parameter "numberOfBytesRx" gibt an, wie viele
///					Bytes empfangen werden sollen. Wenn diese Anzahl erreicht ist, gibt die Funktion
///					"UartGetStatusRxA()" bei Aufruf den Wert "UART_STATUS_RX_FINISHED" zurück (Daten
///					vollständig empfangen). Wird innerhalb des mit dem Parameter "timeOut" übergebenen
///					Zeitfensters kein vollständiges Datenpaket empfangen (= "numberOfBytesRx"), so wird
///					der Empfangsvorgang abgebrochen (z.B. im Hauptprogramm durch Abfrage der Variable
///         "uartRxTimeoutA" auf 0). Da UART eine asynchrone Schnittstelle ist, ist die Funktion
///					"UartReceiveA()" nicht als Funktion zum forcierten Empfangen von Daten zu verstehen,
///					sondern als initialisierende Vorbereitung zum Empfang vom Daten.
///
/// @param  uint16_t numberOfBytesRx, int32_t timeOut
///
/// @return bool operationPerformed
///
//=================================================================================================
extern bool UartReceiveA(uint16_t numberOfBytesRx,
												 int32_t timeOut)
{
		// Ergebnis des Funktionsaufrufes (Empfangsvorgang initiiert / nicht initiiert)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist
		// und die Anzahl der zu empfangenen Bytes die Größe des Software-Puffers
		// nicht überschreitet und mindestens 1 ist
		if ((uartStatusFlagRxA != UART_STATUS_IN_PROGRESS)
				&& (numberOfBytesRx <= UART_SIZE_BUFFER_RX)
				&& numberOfBytesRx)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Timeout-Zähler auf den übergebenen Wert setzen
				uartRxTimeoutA = timeOut;
				// Index zur Verwaltung des Software-Puffers "uartBufferRx[]" auf das erste Element
				// setzen, damit die empfangenen Daten an den Anfang des Puffers kopiert werden
				uartBufferIndexRxA = 0;
				// Menge der zu empfangenen Bytes an die Steuervariable übergeben.
				// Diese koordiniert die restliche Kommunikation in der ISR
				uartBytesToTransferRxA = numberOfBytesRx;
				// Rx-Interrupt einschalten um neue Daten zu empfangen
				SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
				// Rückgabewert auf "true" setzen, um der aufrufenden Stelle
				// zu signalisieren, dass der Empfangsvorgang initiiert wurde
				operationPerformed = true;
		}
		return operationPerformed;
}


//=== Function: UartTransmitA =====================================================================
///
/// @brief  Funktion sendet Daten über UART aus dem Sotware-Puffer "uartBufferTxA[]". Der Parameter
///					"numberOfBytesTx" gibt an, wie viele Bytes gesendet werden sollen. Das erste Byte wird
///					direkt im Funktionsaufruf gesendet, alle weiteren werden in einer ISR versendet.
///
/// @param  uint16_t numberOfBytesTx
///
/// @return bool operationPerformed
///
//=================================================================================================
extern bool UartTransmitA(uint16_t numberOfBytesTx)
{
		// Ergebnis des Funktionsaufrufes (Sendevorgang gestartet / nicht gestartet)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist
		// und die Anzahl der zu sendenen Bytes die Größe des Software-Puffers
		// nicht überschreitet und mindestens 1 ist
		if ((uartStatusFlagTxA != UART_STATUS_IN_PROGRESS)
				&& (numberOfBytesTx <= UART_SIZE_BUFFER_TX)
				&& numberOfBytesTx)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Flag setzen um der aufrufenden Stelle zu signalisieren,
				// dass eine UART-Kommunikation gestartet wurde
				uartStatusFlagTxA = UART_STATUS_IN_PROGRESS;
				// Index zur Verwaltung des Software-Puffers "uartBufferTxA[]" auf das erste Element
				// setzen, damit die zu sendenen Daten vom Anfang des Puffers aus kopiert werden
				uartBufferIndexTxA = 0;
				// Menge der zu sendenen Bytes an die Steuervariable übergeben.
				// Diese koordiniert die restliche Kommunikation in der ISR
				uartBytesToTransferTxA = numberOfBytesTx;
				// Tx-Interrupt einschalten. Interrupt wird ausgelöst, sobald ein Byte
				// aus dem Register SCITXBUF in das Senderegister geshiftet wurde
				SciaRegs.SCICTL2.bit.TXINTENA = 1;
				// Erstes Datenbyte senden
				SciaRegs.SCITXBUF.bit.TXDT = uartBufferTxA[uartBufferIndexTxA];
				uartBufferIndexTxA++;
				// Rückgabewert auf "true" setzen, um der aufrufenden Stelle
				// zu signalisieren, dass der Sendevorgang gestartet wurde
				operationPerformed = true;
		}
		return operationPerformed;
}


//=== Function: UartRxISRA ========================================================================
///
/// @brief  ISR wird aufgerufen, sobald ein Byte über UART (SCI) empfangen wurde. Das Byte wird
///         ausgelesen und im Software-Ppuffer "uartBufferRxA[]" gespeichert, falls dieser nicht
///         voll ist. Anschließend werden alle Interrupt-Flags gelöscht.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void UartRxISRA(void)
{
		// Empfangenes Byte auslesen
		uint16_t dataRx = SciaRegs.SCIRXBUF.bit.SAR;
		// Empfangene Daten solange in den Software-Puffer kopieren, bis dieser voll ist
		if (uartBufferIndexRxA < UART_SIZE_BUFFER_RX)
		{
				uartBufferRxA[uartBufferIndexRxA] = dataRx;
				uartBufferIndexRxA++;
		}
		// Status-Flag setzen, wenn von dem Ruhe-
		// in den Empfangszustand gewechselt wird
		if (uartStatusFlagRxA == UART_STATUS_IDLE)
		{
				uartStatusFlagRxA = UART_STATUS_IN_PROGRESS;
		}

		// Interrupt-Flag der Gruppe 9 löschen (da gehört der INT_SCIA_RX-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK9 = 1;
}


//=== Function: UartTxISRA ========================================================================
///
/// @brief  ISR wird aufgerufen, sobald ein Byte aus dem Register SCITXBUF in das Senderegister
///					geshiftet wurde. Das bedeutet, dass die ISR aufgerufen wird, wenn das zu sendene
///					Byte gerade angefangen wird an dem Pin TxD auszugeben und nicht nach Ende des
///					Sendevorgangs! Sind beim Aufruf der ISR noch weitere Bytes zu senden, so wird
///         das nächste aus dem Software-Puffer "uartBufferTxA[]" gesendet. Andernfalls wird
///         der Tx-Interrupt ausgeschaltet. Anschließend werden alle Interrupt-Flags gelöscht.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void UartTxISRA(void)
{
		// Datenpaket wurde noch nicht vollständig gesendet
		if (uartBufferIndexTxA < uartBytesToTransferTxA)
		{
				// Nächste Byte aus dem Software-Puffer senden
				SciaRegs.SCITXBUF.bit.TXDT = uartBufferTxA[uartBufferIndexTxA];
				uartBufferIndexTxA++;
		}
		// Datenpaket wurde vollständig gesendet
		else
		{
				// Tx-Interrupt ausschalten
				SciaRegs.SCICTL2.bit.TXINTENA = 0;
				// Flag setzen um das Ende der Übertragung zu signalisieren
				uartStatusFlagTxA = UART_STATUS_FINISHED;
		}

		// Interrupt-Flag der Gruppe 9 löschen (da gehört der INT_SCIA_TX-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK9 = 1;
}
