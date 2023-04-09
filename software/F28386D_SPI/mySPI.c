//=================================================================================================
/// @file       mySPI.c
///
/// @brief      Datei enthält Variablen und Funktionen um die SPI-A Schnittstelle eines TMS320F283x
///							als Master zu benutzen. Die Kommunikation ist Interrupt-basiert und kann entweder
///						  Simplex (nur lesen -> "SpiReadA()" oder nur schreiben -> "SpiWriteA()") oder duplex
///             (lesen/schreiben -> "SpiWriteReadA()") erfolgen. Der Status der Kommunikation
///							(Kommunikation aktiv oder nicht aktiv) kann über die Funktion "SpiGetStatusA()"
///             abgefragt werden. Es wird das SPI-A Modul verwendet. Die Module B, C und D können
///							analog zu den hier gezeigten Funktionen verwendet werden.
///
/// @version    V1.2
///
/// @date       12.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "mySPI.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Software-Puffer für die SPI-Kommunikation
uint16_t spiBufferTxA[SPI_SIZE_BUFFER_TX];
uint16_t spiBufferRxA[SPI_SIZE_BUFFER_RX];
// Steuern das Kopieren in und aus den Software-Puffern während der SPI-Kommunikation
uint16_t spiTransferIndexA;
uint16_t spiBytesToTransferA;
// Flag speichert den aktuellen Zustand der SPI-Kommunikation
uint16_t spiStatusFlagA;


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: SpiInitA ==========================================================================
///
/// @brief  Funktion initialisiert GPIO 54 (MOSI), GPIO 55 (MISO), GPIO 56 (CLK) und GPIO 58 (SS)
/// 				als SPI-Pins und das SPI-A Modul als Master mit Taktrate nach übergebenen Funktions-
///					parameter "clock" und 8 Bit Datenlänge. Der SPI-Interrupt wird eingeschaltet und die
///					ISR auf die PIE-Vector-Tabelle gesetzt. Die Konfiguration der SPI Module B, C und D
///					sind analog zu dem hier gezeigten Modul A.
///
/// @param  uint32_t clock
///
/// @return void
///
//=================================================================================================
void SpiInitA(uint32_t clock)
{
    // Register-Schreibschutz aufheben
    EALLOW;

    // GPIO-Sperre für GPIO 54 (MOSI), 55 (MISO), 56 (CLK) und 58 (SS) aufheben
    GpioCtrlRegs.GPBLOCK.bit.GPIO54  = 0;
    GpioCtrlRegs.GPBLOCK.bit.GPIO55 = 0;
    GpioCtrlRegs.GPBLOCK.bit.GPIO56 = 0;
    GpioCtrlRegs.GPBLOCK.bit.GPIO58 = 0;
    // GPIO 54 auf SPI-Funktion setzen (MOSI)
    // Die Zahl in der obersten Zeile der Tabelle gibt den Wert für
    // GPAGMUX (MSB, 2 Bit) + GPAMUX (LSB, 2 Bit) als Dezimalzahl an
    // (siehe S. 1647 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPBGMUX2.bit.GPIO54 = (1 >> 2);
    GpioCtrlRegs.GPBMUX2.bit.GPIO54  = (1 & 0x03);
    // GPIO 54 Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPBPUD.bit.GPIO54 = 1;
    // GPIO 54 Asynchroner Eingang (muss für SPI gesetzt sein)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO54 = 0x03;
    // GPIO 55 auf SPI-Funktion setzen (MISO)
    GpioCtrlRegs.GPBGMUX2.bit.GPIO55 = (1 >> 2);
    GpioCtrlRegs.GPBMUX2.bit.GPIO55  = (1 & 0x03);
    // GPIO 55 Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPBPUD.bit.GPIO55 = 1;
    // GPIO 55 Asynchroner Eingang (muss für SPI gesetzt sein)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO55 = 0x03;
    // GPIO 56 auf SPI-Funktion setzen (CLK)
    GpioCtrlRegs.GPBGMUX2.bit.GPIO56 = (1 >> 2);
    GpioCtrlRegs.GPBMUX2.bit.GPIO56  = (1 & 0x03);
    // GPIO 56 Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPBPUD.bit.GPIO56 = 1;
    // GPIO 56 Asynchroner Eingang (muss für SPI gesetzt sein)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO56 = 0x03;
    // GPIO 58 auf GPIO-Funktionalität setzen
    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = (0 >> 2);
    GpioCtrlRegs.GPBMUX2.bit.GPIO58  = (0 & 0x03);
    // GPIO 58 Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 1;
    // Zustand von GPIO 58 auf high setzen
    GpioDataRegs.GPBSET.bit.GPIO58 = 1;
    // GPIO 58 als Ausgang setzen (SPI SS)
    GpioCtrlRegs.GPBDIR.bit.GPIO58 = 1;
  	// Slave(s) inaktiv setzen
  	SpiDisableSlaveA();

    // Takt für das SPI-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR8.bit.SPI_A = 1;
    __asm(" RPT #4 || NOP");
		// SPI-Modul zum konfigurieren ausschalten
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;
    // Polarität = 0 (Ruhepegel: CLK = 0)
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 0;
    // Phase = 0 (Daten bei der ersten (mit POL = 0 also einer steigenden) Flanke übernehmen)
    SpiaRegs.SPICTL.bit.CLK_PHASE = 0;
    // 8-Bit Datenlänge
    SpiaRegs.SPICCR.bit.SPICHAR = 7;
    // Master-Mode setzen
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;
    // Übertragung aktivieren
    SpiaRegs.SPICTL.bit.TALK = 1;
    // Taktrate setzen
    // (Low-Speed CLK / clock) - 1
    // Low-Speed CLK = 50 MHz (siehe "DeviceInit()")
    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = (50000000 / clock) - 1;
    // Interrupt SPIINT einschalten
    SpiaRegs.SPICTL.bit.SPIINTENA = 1;
    // SPI-Modul wieder einschalten
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;

    // CPU-Interrupts während der Konfiguration global sperren
    DINT;
    // Interrupt-Service-Routinen für den SPI-Interrupt an die
    // entsprechende Stelle (SPIA_RX_INT) der PIE-Vector Table speichern
    PieVectTable.SPIA_RX_INT = &SpiISRA;
    // SPIA_RX-Interrupt freischalten (Zeile 6, Spalte 1 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER6.bit.INTx1 = 1;
    // CPU-Interrupt 6 einschalten (Zeile 6 der Tabelle)
    IER |= M_INT6;
    // CPU-Interrupts nach Konfiguration global wieder freigeben
    EINT;

		// Register-Schreibschutz setzen
		EDIS;

    // SPI Software-Puffer initialisieren
    SpiInitBufferRxA();
    SpiInitBufferTxA();
    // Steuervariablen für die Puffer-Verwaltung initialisieren
    spiTransferIndexA = 0;
    spiBytesToTransferA = 0;
    // Status-Flag für die SPI-Kommunikation auf "idle" setzen
    spiStatusFlagA = SPI_STATUS_IDLE;
}


//=== Function: SpiInitBufferTxA ==================================================================
///
/// @brief	Funktion setzte alle Elemente des Sende-Software-Puffers "spiBufferTxA[]" zu 0
///
/// @param	void
///
/// @return void
///
//=================================================================================================
void SpiInitBufferTxA(void)
{
		// Alle Elemente zu 0 setzen
		for (uint16_t i=0; i<SPI_SIZE_BUFFER_TX; i++)
		{
				spiBufferTxA[i] = 0;
		}
}


//=== Function: SpiInitBufferRxA ==================================================================
///
/// @brief	Funktion setzte alle Elemente des Empfangs-Software-Puffers "spiBufferRxA[]" zu 0
///
/// @param	void
///
/// @return void
///
//=================================================================================================
void SpiInitBufferRxA(void)
{
		// Alle Elemente zu 0 setzen
		for (uint16_t i=0; i<SPI_SIZE_BUFFER_RX; i++)
		{
				spiBufferRxA[i] = 0;
		}
}


//=== Function: SpiGetStatusA ====================================================================
///
/// @brief	Funktion gibt den aktuellen Status der SPI-Kommunikation zurück. Die SPI-Kommunikation
///					ist Interrupt-basiert und kann folgende Zustände annehmen:
///
///					- SPI_STATUS_IDLE              : Es ist keine Kommunikation aktiv
///					- SPI_STATUS_TX_IN_PROGRESS    : Es wurde eine Schreib-Kommunikation gestartet
///					- SPI_STATUS_TX_FINISHED       : Eine Schreib-Kommunikation ist abgeschlossen
///					- SPI_STATUS_RX_IN_PROGRESS    : Es wurde eine Lese-Kommunikation gestartet
///					- SPI_STATUS_RX_FINISHED       : Eine Lese-Kommunikation ist abgeschlossen
///					- SPI_STATUS_TX_RX_IN_PROGRESS : Es wurde eine Lese/Schreib-Kommunikation gestartet
///					- SPI_STATUS_TX_RX_FINISHED    : Eine Lese/Schreib-Kommunikation ist abgeschlossen
///
///					Zum starten einer Kommunikation mit einem Slave muss eine der Funktionen
///					"SpiReadA()", "SpiWriteA()" oder "SpiWriteReadA()" aufgerufen werden
///
/// @param 	uint16_t spiStatusFlag
///
/// @return void
///
//=================================================================================================
uint16_t SpiGetStatusA(void)
{
		return spiStatusFlagA;
}


//=== Function: SpiSetStatusIdleA =================================================================
///
/// @brief	Funktion setzt das Status-Flag auf "idle" und gibt "true" zurück, falls die vorherige
///					Kommunikation abgeschlossen ist. Ist noch eine Kommunikation aktiv, wird das Flag nicht
///					verändert und es wird "false" zurück gegeben.
///
/// @param	void
///
/// @return bool flagSetToIdle
///
//=================================================================================================
bool SpiSetStatusIdleA(void)
{
		bool flagSetToIdle = false;
		// Staus-Flag nur auf "idle" setzen, falls eine
		// vorherige Kommunikation abgeschlossen ist
		if ((spiStatusFlagA == SPI_STATUS_RX_FINISHED)
				|| (spiStatusFlagA == SPI_STATUS_TX_FINISHED)
				|| (spiStatusFlagA == SPI_STATUS_TX_RX_FINISHED))
		{
				spiStatusFlagA = SPI_STATUS_IDLE;
				flagSetToIdle  = true;
		}
		return flagSetToIdle;
}


//=== Function: SpiWriteA =========================================================================
///
/// @brief	Funktion schreibt über SPI Daten an einen Slave. Die zu sendenen Daten werden aus dem
///					Software-Puffer "spiBufferTxA[]" kopiert. Der Parameter "slaveSelect" gibt an mit
///					welchem Slave kommuniziert werden soll. Dafür kann einer der folgenden Parameter
///					verwendet werden:
///
///					- SPI_SELECT_SLAVE1
///					- SPI_SELECT_SLAVE2
///					- SPI_SELECT_SLAVE3
///					- SPI_SELECT_SLAVE4
///
///					Der Parameter "numberOfBytes" gibt die Anzahl der zu sendenen Bytes an
///
/// @param 	uint16_t slaveSelect, uint16_t numberOfBytes
///
/// @return bool operationPerformed
///
//=================================================================================================
bool SpiWriteA(uint16_t slaveSelect,
							 uint16_t numberOfBytes)
{
		// Ergebnis des Funktionsaufrufes (Schreibvorgang gestartet / nicht gestartet)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist
		// und die Anzahl der zu schreibenden Bytes die Größe des Software-
		// Puffers nicht überschreitet und mindestens 1 ist
		if ((spiStatusFlagA != SPI_STATUS_TX_IN_PROGRESS)
				&& (spiStatusFlagA != SPI_STATUS_RX_IN_PROGRESS)
				&& (spiStatusFlagA != SPI_STATUS_TX_RX_IN_PROGRESS)
				&& (numberOfBytes <= SPI_SIZE_BUFFER_TX)
				&& numberOfBytes)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Flag setzen um der aufrufenden Stelle zu signalisieren,
				// dass eine SPI-Kommunikation gestartet wurde
				spiStatusFlagA = SPI_STATUS_TX_IN_PROGRESS;
				// Index zur Verwaltung des Software-Puffers "spiBufferTxA[]" auf das erste Element setzen,
				// damit die zu sendenen Daten vom Anfang des Puffers kopiert werden
				spiTransferIndexA = 0;
				// Menge der zu sendenen Bytes an die Steuervariable übergeben.
				// Diese koordiniert die Kommunikation in der ISR
				spiBytesToTransferA = numberOfBytes;
				// Slave aktivieren, an den gesendet werden soll
				if (slaveSelect & SPI_SELECT_SLAVE1)
				{
						SPI_ENABLE_SLAVE1;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE1)
				{
						SPI_ENABLE_SLAVE2;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE1)
				{
						SPI_ENABLE_SLAVE3;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE1)
				{
						SPI_ENABLE_SLAVE4;
				}
				// Ggf. warten bis der Slave bereit ist
				//DEVICE_DELAY_US(10);
				// Erstes Datenbyte senden.
				// 8-Bit Datenformat, Daten müssen linksbündig sein
				SpiaRegs.SPIDAT = (spiBufferTxA[spiTransferIndexA] << 8);
				spiTransferIndexA++;
		}
		return operationPerformed;
}


//=== Function: SpiReadA ==========================================================================
///
/// @brief	Funktion liest über SPI Daten von einem Slave. Die empfangenen Daten werden in den
///					Software-Puffer "spiBufferRxA[]" gespeichert. Der Parameter "slaveSelect" gibt an mit
///					welchem Slave kommuniziert werden soll. Dafür kann einer der folgenden Parameter
///					verwendet werden:
///
///					- SPI_SELECT_SLAVE1
///					- SPI_SELECT_SLAVE2
///					- SPI_SELECT_SLAVE3
///					- SPI_SELECT_SLAVE4
///
///					Der Parameter "numberOfBytes" gibt die Anzahl der zu empfangenen Bytes an
///
/// @param	uint16_t slaveSelect, uint16_t numberOfBytes
///
/// @return bool operationPerformed
///
//=================================================================================================
bool SpiReadA(uint16_t slaveSelect,
						  uint16_t numberOfBytes)
{
		// Ergebnis des Funktionsaufrufes (Lesevorgang gestartet / nicht gestartet)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist
		// und die Anzahl der zu lesenden Bytes die Größe des Software-
		// Puffers nicht überschreitet und mindestens 1 ist
		if ((spiStatusFlagA != SPI_STATUS_TX_IN_PROGRESS)
				&& (spiStatusFlagA != SPI_STATUS_RX_IN_PROGRESS)
				&& (spiStatusFlagA != SPI_STATUS_TX_RX_IN_PROGRESS)
				&& (numberOfBytes <= SPI_SIZE_BUFFER_RX)
				&& numberOfBytes)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Flag setzen um der aufrufenden Stelle zu signalisieren,
				// dass eine SPI-Kommunikation gestartet wurde
				spiStatusFlagA = SPI_STATUS_RX_IN_PROGRESS;
				// Index zur Verwaltung des Software-Puffers "spiBufferRxA[]" auf das erste Element setzen,
				// damit die empfangenen Daten an den Anfang des Puffers geschieben werden
				spiTransferIndexA = 0;
				// Menge der zu empfangenen Bytes an die Steuervariable übergeben.
				// Diese koordiniert die Kommunikation in der ISR
				spiBytesToTransferA = numberOfBytes;
				// Slave aktivieren, von dem gelesen werden soll
				if (slaveSelect & SPI_SELECT_SLAVE1)
				{
						SPI_ENABLE_SLAVE1;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE2)
				{
						SPI_ENABLE_SLAVE2;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE3)
				{
						SPI_ENABLE_SLAVE3;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE4)
				{
						SPI_ENABLE_SLAVE4;
				}
				// Ggf. warten bis der Slave bereit ist
				//DEVICE_DELAY_US(10);
				// Erstes Dummy-Datenbyte senden
				SpiaRegs.SPIDAT = SPI_DUMMY_DATA;
				spiTransferIndexA++;
		}
		return operationPerformed;
}


//=== Function: SpiWriteReadA =====================================================================
///
/// @brief	Funktion schreibt über SPI Daten an einen Slave und liest gleichzeitig welche von ihm.
///					Die zu sendenen Daten werden aus dem Software-Puffer "spiBufferTxA[]" kopiert, die
///					empfangen Daten werden in den Software-Puffer "spiBufferRxA[]" gespeichert. Der Parameter
///					"slaveSelect" gibt an mit welchem Slave kommuniziert werden soll. Dafür kann einer der
///					folgenden Parameter verwendet werden:
///
///					- SPI_SELECT_SLAVE1
///					- SPI_SELECT_SLAVE2
///					- SPI_SELECT_SLAVE3
///					- SPI_SELECT_SLAVE4
///
///					Der Parameter "numberOfBytes" gibt die Anzahl der zu sendenen/empfangenen Bytes an
///
/// @param	uint16_t slaveSelect, uint16_t numberOfBytes
///
/// @return bool operationPerformed
///
//=================================================================================================
bool SpiWriteReadA(uint16_t slaveSelect,
									 uint16_t numberOfBytes)
{
		// Ergebnis des Funktionsaufrufes (Schreib-Lesevorgang gestartet / nicht gestartet)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist
		// und die Anzahl der zu schreibenden und lesenden Bytes die Größe der
		// Software-Puffer nicht überschreitet und mindestens 1 ist
		if ((spiStatusFlagA != SPI_STATUS_TX_IN_PROGRESS)
				&& (spiStatusFlagA != SPI_STATUS_RX_IN_PROGRESS)
				&& (spiStatusFlagA != SPI_STATUS_TX_RX_IN_PROGRESS)
				&& (numberOfBytes <= SPI_SIZE_BUFFER_TX)
				&& (numberOfBytes <= SPI_SIZE_BUFFER_RX)
				&& numberOfBytes)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Flag setzen um der aufrufenden Stelle zu signalisieren,
				// dass eine SPI-Kommunikation gestartet wurde
				spiStatusFlagA = SPI_STATUS_TX_RX_IN_PROGRESS;
				// Index zur Verwaltung der Software-Puffer "spiBufferRxA[]" und "spiBufferTxA[]"
				// auf das erste Element setzen, damit die empfangenen bzw. gesendeten Daten an
				// den Anfang der Puffer kopiert bzw. gelesen werden
				spiTransferIndexA = 0;
				// Menge der zu sendenen Bytes an die Steuervariable übergeben.
				// Diese koordiniert die Kommunikation in der ISR
				spiBytesToTransferA = numberOfBytes;
				// Slave aktivieren, mit dem kommuniziert werden soll
				if (slaveSelect & SPI_SELECT_SLAVE1)
				{
						SPI_ENABLE_SLAVE1;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE2)
				{
						SPI_ENABLE_SLAVE2;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE3)
				{
						SPI_ENABLE_SLAVE3;
				}
				else if (slaveSelect & SPI_SELECT_SLAVE4)
				{
						SPI_ENABLE_SLAVE4;
				}
				// Ggf. warten bis der Slave bereit ist
				//DEVICE_DELAY_US(10);
				// Erstes Datenbyte senden.
				// 8-Bit Datenformat, Daten müssen linksbündig sein
				SpiaRegs.SPIDAT = (spiBufferTxA[spiTransferIndexA] << 8);
				spiTransferIndexA++;
		}
		return operationPerformed;
}


//=== Function: SpiDisableSlaveA ==================================================================
///
/// @brief	Funktion setzt den aktiven Slave durch setzen der
///         entsprechenden SS-Leitung auf logisch 1 inaktiv
///
/// @param	void
///
/// @return void
///
//=================================================================================================
void SpiDisableSlaveA(void)
{
		// Aktiven Slaves abwählen
		if (SPI_SLAVE1_IS_ENABLED)
		{
				SPI_DISABLE_SLAVE1;
		}
		else if (SPI_SLAVE2_IS_ENABLED)
		{
				SPI_DISABLE_SLAVE2;
		}
		else if (SPI_SLAVE3_IS_ENABLED)
		{
				SPI_DISABLE_SLAVE3;
		}
		else if (SPI_SLAVE4_IS_ENABLED)
		{
				SPI_DISABLE_SLAVE4;
		}
}


//=== Function: SpiISRA ===========================================================================
///
/// @brief	Funktion wird aufgerufen, sobald ein Byte über SPI empfangen wurde
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void SpiISRA(void)
{
		// Empfangene Daten unabhängig von der Kommunikationsart auslesen (Schreib-, Lese- oder
		// Lese/Schreib-Befehl), damit kein Overflow-Interrupt auftritt. Das Auslesen löscht
		// gleichzeitig das Flag "INT_FLAG" im Statusregister
		uint16_t dataRx = SpiaRegs.SPIRXBUF;

		// Unterscheidung nach Kommunikationsart:
		// Schreib-Betrieb
		if (spiStatusFlagA == SPI_STATUS_TX_IN_PROGRESS)
		{
				// Übertragung ist noch nicht abgeschlossen
				if (spiTransferIndexA < spiBytesToTransferA)
				{
						// Nächstes Daten-Byte senden.
						// 8-Bit Datenformat, Daten müssen linksbündig sein
						SpiaRegs.SPIDAT = (spiBufferTxA[spiTransferIndexA] << 8);
						spiTransferIndexA++;
				}
				// Übertragung ist abgeschlossen (letztes Byte wurde gesendet)
				else
				{
						// Flag setzen um das Ende der Übertragung zu signalisieren
						spiStatusFlagA = SPI_STATUS_TX_FINISHED;
				}
		}
		// Lese-Betrieb
		else if (spiStatusFlagA == SPI_STATUS_RX_IN_PROGRESS)
		{
				// Übertragung ist noch nicht abgeschlossen
				if (spiTransferIndexA < spiBytesToTransferA)
				{
						// Empfangenes Byte in den Software-Empfangspuffer kopieren.
						// Index-1, weil immer zuerst ein Byte gesendet wird (Dummy)
						// und erst danach das dazugehörige Byte ausgelesen werden kann.
						// 8-Bit Datenformat
						spiBufferRxA[spiTransferIndexA-1] = (dataRx & 0x00FF);
						spiTransferIndexA++;
						// Dummy-Daten senden um ein weiteres Byte vom Slave zu empfangen.
						SpiaRegs.SPIDAT = SPI_DUMMY_DATA;
				}
				// Übertragung ist abgeschlossen (letztes Byte wurde empfangen)
				else
				{
						// Empfangenes (letztes) Byte in den Software-Empfangspuffer kopieren.
						// 8-Bit Datenformat
						spiBufferRxA[spiTransferIndexA-1] = (dataRx & 0x00FF);
						// Flag setzen um das Ende der Übertragung zu signalisieren
						spiStatusFlagA = SPI_STATUS_RX_FINISHED;
				}
		}
		// Schreib-Lese-Betrieb
		else if (spiStatusFlagA == SPI_STATUS_TX_RX_IN_PROGRESS)
		{
				// Übertragung ist noch nicht abgeschlossen
				if (spiTransferIndexA < spiBytesToTransferA)
				{
						// Empfangenes Byte in den Software-Empfangspuffer kopieren.
						// Index-1, weil immer zuerst ein Byte gesendet wird und erst
						// danach das dazugehörige Byte ausgelesen werden kann.
						// 8-Bit Datenformat
						spiBufferRxA[spiTransferIndexA-1] = (dataRx & 0x00FF);
						// Nächstes Daten-Byte senden.
						// 8-Bit Datenformat, Daten müssen linksbündig sein
						SpiaRegs.SPIDAT = (spiBufferTxA[spiTransferIndexA] << 8);
						spiTransferIndexA++;
				}
				// Übertragung ist abgeschlossen (letztes Byte wurde empfangen bzw. gesendet)
				else
				{
						// Empfangenes (letztes) Byte in den Software-Empfangspuffer kopieren.
						// 8-Bit Datenformat
						spiBufferRxA[spiTransferIndexA-1] = (dataRx & 0x00FF);
						// Flag setzen um das Ende der Übertragung zu signalisieren
						spiStatusFlagA = SPI_STATUS_TX_RX_FINISHED;
				}
		}

    // Overflow Interrupt-Flag im SPI-Modul löschen.
		// Im fehlerfreien Betrieb wird dieses jedoch nie gesetzt
		SpiaRegs.SPISTS.bit.OVERRUN_FLAG = 1;
		// Interrupt-Flag der Gruppe 6 löschen (da gehört der SPI-Interrupt zu)
    PieCtrlRegs.PIEACK.bit.ACK6 = 1;
}


