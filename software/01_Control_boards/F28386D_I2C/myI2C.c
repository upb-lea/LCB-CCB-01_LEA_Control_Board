//=================================================================================================
/// @file       myI2C.c
///
/// @brief      Datei enthält Variablen und Funktionen um die I2C-Schnittstelle (Modul A) eines
///							TMS320F2838x als Master zu benutzen. Die Kommunikation ist Interrupt-basiert.
///						  Der Status der Kommunikation (Kommunikation aktiv oder nicht aktiv) kann über
///             die Funktion "I2cGetStatus()" abgefragt werden. Es wird das SPI-A Modul verwendet.
///							Das Modul B kann analog zu den hier gezeigten Funktionen verwendet werden.
///
/// @version    V1.2
///
/// @date       14.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myI2C.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Software-Puffer für die I2C-Kommunikation
uint16_t i2cBufferWriteA[I2C_SIZE_BUFFER_WRITE];
uint16_t i2cBufferReadA[I2C_SIZE_BUFFER_READ];
// Steuern das Kopieren in und aus den Software-Puffern während der I2C-Kommunikation
uint16_t i2cBufferIndexWriteA;
uint16_t i2cBufferIndexReadA;
// Übernimmt die Anzahl der zu lesenden Bytes beim Aufruf der Funktion "I2cWriteReadA()"
// und übergibt sie dem Register I2CCNT nach einer wiederholten START-Bedingung um die
// gewünschte Anzahl von Bytes vom Slave zu lesen
uint16_t i2cBytesToReadAfterRSA;
// Flag speichert den aktuellen Zustand der I2C-Kommunikation
uint16_t i2cStatusFlagA;


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: I2cInitA ==========================================================================
///
/// @brief  Funktion initialisiert GPIO 8 (SCL) und GPIO 10 (SDA) als I²C-Pins, das I²C-Modul
///         als Master mit 100 kHz oder 400 kHz Clock, die Adressierung von Slaves auf 7 Bit und
///					die Datengröße auf 8 Bit. Der I2C-Interrupt wird eingeschaltet und die ISR auf die
///					PIE-Vector-Tabelle gesetzt.
///
/// @param  uint32_t clock
///
/// @return void
///
//=================================================================================================
void I2cInitA(uint32_t clock)
{
    // Register-Schreibschutz aufheben
    EALLOW;

    // GPIO-Sperre für GPIO 0 aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO0 = 0;
    // GPIO 0 auf I2C-Funktion setzen (SDA)
    // Die Zahl in der obersten Zeile der Tabelle gibt den Wert für
    // GPAGMUX (MSB, 2 Bit) + GPAMUX (LSB, 2 Bit) als Dezimalzahl an
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = (6 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO0  = (6 & 0x03);
    // GPIO 0 Pull-Up-Widerstand aktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0;
    // GPIO 0 Asynchroner Eingang (muss für I2C gesetzt sein)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO0 = 0x03;
    // GPIO-Sperre für GPIO 1 aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO1 = 0;
    // GPIO 1 auf I2C-Funktion setzen (SCL)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = (6 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO1  = (6 & 0x03);
    // GPIO 1 Pull-Up-Widerstand aktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0;
    // GPIO 1 Asynchroner Eingang (muss für I2C gesetzt sein)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0x03;

    // Takt für das I2C-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR9.bit.I2C_A = 1;
    __asm(" RPT #4 || NOP");
		// I2C-Modul ausschalten, um es zu konfigurieren
		I2caRegs.I2CMDR.bit.IRS = 0;
		// Initialisierung als Master
		I2caRegs.I2CMDR.bit.MST = 1;
		// Siehe S. 3629 ff. Reference Manual TMS320F2838x:
		// Vorteiler auf 20 setzen (Teiler = IPSC+1) um
		// einen I2C-Modul-Takt von 10 MHz zu erhalten
		// (I2C-Modul-Takt = SYSCLK / (IPSC+1) = 200 MHz / (IPSC+1))
		I2caRegs.I2CPSC.bit.IPSC = 19;
		// SCL-Takt für 400 kHz Fastmode initialisieren.
		// Entweder ist die Gleichung (26) oder der Wert für d in Tabelle
		// 33-1 falsch (siehe S. 3630 Reference Manual TMS320F2838x).
		// Jedenfalls lassen sich mit den beiden Informationen die Werte
		// für die Register I2CCLKH und I2CCLKL nicht korrekt berechnen.
		// Durch Ausprobieren ergaben sich die unteren Werte
		if (clock == I2C_CLOCK_400_KHZ)
		{
				// Zeit setzen, in der SCL 1 ist
				I2caRegs.I2CCLKH = 5;
				// Zeit setzen, in der SCL 0 ist
				I2caRegs.I2CCLKL = 4;
		}
		// Standardmäßig mit 100 kHz SCL-Takt initialisieren
		else
		{
				// Zeit setzen, in der SCL 1 ist
				I2caRegs.I2CCLKH = 42;
				// Zeit setzen, in der SCL 0 ist
				I2caRegs.I2CCLKL = 42;
		}
		// Adressformat: 7 Bit Adresse
		I2caRegs.I2CMDR.bit.XA  = 0;
		I2caRegs.I2CMDR.bit.FDF = 0;
		// Datenformat: 8 Bit Daten pro Byte
		I2caRegs.I2CMDR.bit.BC = I2C_DATA_BITS_8;
		// Interrupts einschalten:
		// Interrupt wenn Datenbyte vom (Hardware) Sende-Puffer in Ausgangs-Register
		// geshiftet wurde (zu diesem Zeitpunkt sind die Daten noch nicht gesendet!)
		I2caRegs.I2CIER.bit.XRDY = 1;
		// Interrupt wenn Datenbyte vom Empfangs-Register in
		// den (Hardware) Empfangs-Puffer geshiftet wurde
		I2caRegs.I2CIER.bit.RRDY = 1;
		// Interrupt wenn eine STOP-Bedingung erkannt wurde
		I2caRegs.I2CIER.bit.SCD = 1;
		// Interrupt wenn ein NACK empfangen wurde
		I2caRegs.I2CIER.bit.NACK = 1;
		// I2C-Modul wieder einschalten
		I2caRegs.I2CMDR.bit.IRS = 1;

		// CPU-Interrupts während der Konfiguration global sperren
    DINT;
		// Interrupt-Service-Routinen für den I2C-Interrupt an die
    // entsprechende Stelle (I2CA_INT) der PIE-Vector Table speichern
    PieVectTable.I2CA_INT = &I2cISRA;
    // I2CA-Interrupt freischalten (Zeile 8, Spalte 1 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER8.bit.INTx1 = 1;
    // CPU-Interrupt 8 einschalten (Zeile 8 der Tabelle)
    IER |= M_INT8;
    // Interrupts global einschalten
    EINT;

		// Register-Schreibschutz setzen
		EDIS;

    // Software-Puffer inititalisieren
    I2cInitBufferReadA();
    I2cInitBufferWriteA();
    // Steuervariablen für die Puffer-Verwaltung initialisieren
    i2cBufferIndexReadA    = 0;
    i2cBufferIndexWriteA   = 0;
    i2cBytesToReadAfterRSA = 0;
    // Status-Flag für die I2C-Kommunikation auf "idle" setzen
    i2cStatusFlagA = I2C_STATUS_IDLE;
}


//=== Function: I2cInitBufferReadA ================================================================
///
/// @brief  Funktion initialisiert alle Elemente des I2C Software-Lesepuffers zu 0.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void I2cInitBufferReadA(void)
{
    for(uint16_t i=0; i<I2C_SIZE_BUFFER_READ; i++)
    {
        i2cBufferReadA[i] = 0;
    }
}


//=== Function: I2cInitBufferWriteA ===============================================================
///
/// @brief  Funktion initialisiert alle Elemente des I2C Software-Schreibpuffers zu 0.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void I2cInitBufferWriteA(void)
{
    for(uint16_t i=0; i<I2C_SIZE_BUFFER_WRITE; i++)
    {
    		i2cBufferWriteA[i] = 0;
    }
}


//=== Function: I2cGetStatusA =====================================================================
///
/// @brief	Funktion gibt den aktuellen Status der I2C-Kommunikation zurück. Die I2C-Kommunikation
///					ist Interrupt-basiert und kann folgende Zustände annehmen:
///
///					- I2C_STATUS_IDLE       : Es ist keine Kommunikation aktiv
///					- I2C_STATUS_IN_PROGRESS: Es wurde Kommunikation gestartet
///					- I2C_STATUS_FINISHED   : Eine Kommunikation ist abgeschlossen
///					- I2C_STATUS_ERROR      : Es ist ein Fehler während der Kommunikation aufgetreten
///
///					Zum starten einer Kommunikation mit einem Slave muss eine der Funktionen
///				  "I2cWriteA()", "I2cReadA()" oder "I2cWriteReadA()" aufgerufen werden
///
/// @param 	uint16_t i2cStatusFlag
///
/// @return void
///
//=================================================================================================
extern uint16_t I2cGetStatusA(void)
{
		uint16_t status = i2cStatusFlagA;
		// Solange noch keine STOP-Bedingung vollständig gesendet wurde, gilt der Status der
		// Kommunikation unabhängig vom Wert von "i2cStatusFlagA" als "aktiv/in Bearbeitung".
		// Das Bit STP wird leicht verzögert nach dem Senden einer STOP-Bedingung gelöscht,
		// sodass erst nach Aussendung der vollständigen STOP-Bedingung die Kommunikation
		// wieder freigegeben wird
		if (I2caRegs.I2CMDR.bit.STP)
		{
				status = I2C_STATUS_IN_PROGRESS;
		}
		return status;
}


//=== Function: I2cSetStatusIdleA =================================================================
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
extern bool I2cSetStatusIdleA(void)
{
		bool flagSetToIdle = false;
		// Staus-Flag nur auf "idle" setzen, falls eine
		// vorherige Kommunikation abgeschlossen ist
		if (i2cStatusFlagA != I2C_STATUS_IN_PROGRESS)
		{
				i2cStatusFlagA = I2C_STATUS_IDLE;
				flagSetToIdle = true;
		}
		return flagSetToIdle;
}


//=== Function: I2cWriteA =========================================================================
///
/// @brief  Funktion schreibt Daten über I²C auf einen Slave (Master Transmitter Mode). Der erste
///					Parameter ist die 7-Bit Adresse des Slaves, an den die Daten gesendet werden sollen.
///					Der zweite Parameter gibt die Anzahl der zu sendenen Bytes an (ohne die Slave-Adresse).
///					Es können max. 65.535 Bytes geschrieben werden. Der Rückgabwert ist "true" falls die
///					Kommunikation gestartet wurde (keine vorangegangene Kommunikation ist aktiv und der Bus
///					ist frei), andernfalls ist er "false". Die zu schreibenen Daten werden aus dem Software-
///					Puffer "i2cBufferWriteA[]" kopiert.
///
/// @param  uint16_t slaveAddress, uint16_t numberOfBytes
///
/// @return bool operationPerformed
///
//=================================================================================================
bool I2cWriteA(uint16_t slaveAddress,
						   uint16_t numberOfBytes)
{
		// Ergebnis des Funktionsaufrufes (Vorgang gestartet / nicht gestartet)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist,
		// der Bus frei ist, zuvor eine STOP-Bedingung gesendet wurde und die
		// Anzahl der zu schreibenden Bytes die Größe des Software-Puffers nicht
		// überschreitet und mindestens 1 ist
		if ((i2cStatusFlagA != I2C_STATUS_IN_PROGRESS)
				&& !I2caRegs.I2CSTR.bit.BB
				&& !I2caRegs.I2CMDR.bit.STP
				&& (numberOfBytes <= I2C_SIZE_BUFFER_WRITE)
				&& numberOfBytes)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Status-Flag setzen um der aufrufenden Stelle zu signalisieren,
				// dass eine I2C-Kommunikation gestartet wurde
				i2cStatusFlagA = I2C_STATUS_IN_PROGRESS;
				// Master-Transmitter Mode setzen
				I2caRegs.I2CMDR.bit.MST = 1;
				I2caRegs.I2CMDR.bit.TRX = 1;
				// Slave-Adresse setzen
				I2caRegs.I2CSAR.bit.SAR = slaveAddress;
				// Index zur Verwaltung des Software-Puffers "i2cBufferWriteA[]"
				// auf das erste Element setzen, damit die auf den Slave zu
				// schreibenen Daten vom Anfang des Puffers kopiert werden
				i2cBufferIndexWriteA = 0;
				// Anzahl der zu schreibenden Bytes setzen (Adress-Byte zählt nicht dazu)
				I2caRegs.I2CCNT = numberOfBytes;
				// Übertragung starten
				I2caRegs.I2CMDR.bit.STT = 1;
				// STOP-Bedingung senden, nachdem alle Bytes auf den Slave geschrieben wurden
				I2caRegs.I2CMDR.bit.STP = 1;
		}
		return operationPerformed;
}


//=== Function: I2cReadA ==========================================================================
///
/// @brief  Funktion liest Daten über I²C von einem Slave (Master Receiver Mode). Der erste
///					Parameter ist die 7-Bit Adresse des Slaves, von dem die Daten gelesen werden sollen.
///					Der zweite Parameter gibt die Anzahl der zu lesenden Bytes an (ohne die Slave-Adresse).
///					Es können max. 65.535 Bytes gelesen werden. Der Rückgabwert ist "true" falls die
///					Kommunikation gestartet wurde (keine vorangegangene Kommunikation ist aktiv und der Bus
///					ist frei), andernfalls ist er "false". Die gelesenen Daten werden in den Software-Puffer
///					"i2cBufferReadA[]" kopiert.
///
/// @param  uint16_t slaveAddress, uint16_t numberOfBytes
///
/// @return bool operationPerformed
///
//=================================================================================================
bool I2cReadA(uint16_t slaveAddress,
						  uint16_t numberOfBytes)
{
		// Ergebnis des Funktionsaufrufes (Vorgang gestartet/nicht gestartet)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist,
		// der Bus frei ist, zuvor eine STOP-Bedingung gesendet wurde und die
		// Anzahl der zu lesenden Bytes die Größe des Software-Puffers nicht
		// überschreitet und mindestens 1 ist
		if ((i2cStatusFlagA != I2C_STATUS_IN_PROGRESS)
				&& !I2caRegs.I2CSTR.bit.BB
				&& !I2caRegs.I2CMDR.bit.STP
				&& (numberOfBytes <= I2C_SIZE_BUFFER_READ)
				&& numberOfBytes)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Flag setzen um der aufrufenden Stelle zu signalisieren,
				// dass eine I2C-Kommunikation gestartet wurde
				i2cStatusFlagA = I2C_STATUS_IN_PROGRESS;
				// Master-Receiver Mode setzen
				I2caRegs.I2CMDR.bit.MST = 1;
				I2caRegs.I2CMDR.bit.TRX = 0;
				// Slave-Adresse setzen
				I2caRegs.I2CSAR.bit.SAR = slaveAddress;
				// Index zur Verwaltung des Software-Puffers "i2cBufferReadA[]"
				// auf das erste Element setzen, damit die vom Slave gelesenen
				// Daten vom Anfang beginnend in den Puffers kopiert werden
				i2cBufferIndexReadA = 0;
				// Anzahl der zu lesenden Bytes setzen (Adress-Byte zählt nicht dazu)
				I2caRegs.I2CCNT = numberOfBytes;
				// Übertragung starten
				I2caRegs.I2CMDR.bit.STT = 1;
				// STOP-Bedingung senden, nachdem alle Bytes vom Slave gelesen wurden
				I2caRegs.I2CMDR.bit.STP = 1;
		}
		return operationPerformed;
}


//=== Function: I2cWriteReadA =====================================================================
///
/// @brief  Funktion schreibt und liest anschließend Daten über I²C von einem Slave (Master
///					Transmitter + Receiver Mode). Der erste Parameter ist die 7-Bit Adresse des Slaves,
///					mit dem kommuniziert werden soll. Der zweite Parameter gibt die Anzahl der zu
///					schreibenden Bytes an, der Dritte Parameter die Anzahl der zu lesenden Bytes (beides
///					jeweils ohne die Slave-Adresse). Es können max. 65.535 Bytes geschreiben bzw. gelesen
///					werden. Der Rückgabwert ist "true" falls die Kommunikation gestartet wurde (keine
///					vorangegangene Kommunikation ist aktiv und der Bus ist frei), andernfalls ist er
///					"false". Die zu schreibenen Daten werden aus dem Software-Puffer "i2cBufferWriteA[]"
///					kopiert. Die gelesenen Daten werden in den Software-Puffer "i2cBufferReadA[]" kopiert.
///
/// @param  uint16_t slaveAddress, uint16_t numberOfBytesWrite, uint16_t numberOfBytesRead
///
/// @return bool operationPerformed
///
//=================================================================================================
bool I2cWriteReadA(uint16_t slaveAddress,
						       uint16_t numberOfBytesWrite,
									 uint16_t numberOfBytesRead)
{
		// Ergebnis des Funktionsaufrufes (Vorgang gestartet / nicht gestartet)
		bool operationPerformed = false;
		// Vorgang nur starten falls keine vorherige Kommunikation aktiv ist,
		// der Bus frei ist, zuvor eine STOP-Bedingung gesendet wurde und die
		// Anzahl der zu schreibenden und lesenden Bytes die Größe der Software-
		// Puffer nicht überschreitet und mindesten 1 ist
		if ((i2cStatusFlagA != I2C_STATUS_IN_PROGRESS)
				&& !I2caRegs.I2CSTR.bit.BB
				&& !I2caRegs.I2CMDR.bit.STP
				&& (numberOfBytesWrite <= I2C_SIZE_BUFFER_WRITE)
				&& (numberOfBytesRead  <= I2C_SIZE_BUFFER_READ)
				&& numberOfBytesWrite
				&& numberOfBytesRead)
		{
				// Rückgabewert auf "Operation durchgeführt" setzen
				operationPerformed = true;
				// Flag setzen um der aufrufenden Stelle zu signalisieren,
				// dass eine I2C-Kommunikation gestartet wurde
				i2cStatusFlagA = I2C_STATUS_IN_PROGRESS;
				// Master-Transmitter Mode setzen
				I2caRegs.I2CMDR.bit.MST = 1;
				I2caRegs.I2CMDR.bit.TRX = 1;
				// Slave-Adresse setzen
				I2caRegs.I2CSAR.bit.SAR = slaveAddress;
				// Slave-Adresse setzen
				// Index zur Verwaltung des Software-Puffers "i2cBufferWriteA[]"
				// auf das erste Element setzen, damit die auf den Slave zu
				// schreibenen Daten vom Anfang des Puffers kopiert werden
				i2cBufferIndexWriteA = 0;
				// Index zur Verwaltung des Software-Puffers "i2cBufferReadA[]"
				// auf das erste Element setzen, damit die vom Slave gelesenen
				// Daten vom Anfang beginnend in den Puffers kopiert werden
				i2cBufferIndexReadA  = 0;
				// Anzahl der zu schreibenden Bytes setzen (Adress-Byte zählt nicht dazu)
				I2caRegs.I2CCNT = numberOfBytesWrite;
				// Anzahl der zu lesenden Bytes setzen (Adress-Byte zählt nicht dazu).
				// Dieser Wert wird nach Ende der Schreib-Operation in der ISR in das
				// Register I2CCNT geschrieben
				i2cBytesToReadAfterRSA = numberOfBytesRead;
				// ARDY-Interrupt einschalten damit das Ende der
				// Schreib-Operation detektiert werden kann
				I2caRegs.I2CIER.bit.ARDY = 1;
				// Übertragung starten
				I2caRegs.I2CMDR.bit.STT = 1;
		}
		return operationPerformed;
}


//=== Function: I2cISRA ===========================================================================
///
/// @brief	Funktion wird aufgerufen, wenn eine STOP-Benung auf dem Bus gesendet, ein NACK
///					empfangen, ein Byte vom Empfangs-Register in den (Hardware) Empfangs-Puffer geshiftet,
///					ein Byte vom (Hardware) Sende-Puffer in das Sende-Register geshiftet oder der interne
///					Zähler des I2C-Moduls, welcher die Bytes zählt, auf 0 gegangen ist und keine STOP-
///					Bedingung gesendet wurde.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void I2cISRA(void)
{
		// Es wurde eine STOP-Bedingung erkannt
		if (I2caRegs.I2CSTR.bit.SCD)
		{
				// STOP-Flag löschen
				I2caRegs.I2CSTR.bit.SCD = 1;
				// Status-Flag setzen um das Ende der Übertragung zu
				// signalisieren, falls kein Fehler aufgetreten ist
				if (i2cStatusFlagA == I2C_STATUS_IN_PROGRESS)
				{
						i2cStatusFlagA = I2C_STATUS_FINISHED;
				}
		}
		// Der Master-Receiver Mode ist aktiv, ein Datenbyte wurde vom
		// Empfangs-Register in den (Hardware) Empfangs-Puffer geshiftet
		// und es wurde noch nicht alle Bytes vom Slave gelesen
		else if (!I2caRegs.I2CMDR.bit.TRX
						 && I2caRegs.I2CSTR.bit.RRDY
						 && (i2cBufferIndexReadA < I2caRegs.I2CCNT))
		{
				// Empfangenes Datenbyte in den (Software) Empfangs-Puffer schreiben
				i2cBufferReadA[i2cBufferIndexReadA] = I2caRegs.I2CDRR.bit.DATA;
				i2cBufferIndexReadA++;
		}
		// Der Master-Transmitter Mode ist aktiv, ein Datenbyte wurde
		// vom (Hardware) Sende-Puffer in das Sende-Register geshiftet
		// und es wurde noch nicht alle Bytes auf den Slave geschrieben
		else if (I2caRegs.I2CMDR.bit.TRX
						 && I2caRegs.I2CSTR.bit.XRDY
						 && (i2cBufferIndexWriteA < I2caRegs.I2CCNT))
		{
				// Nächstes zu sendenes Datenbyte in den Sende-Puffer schreiben
				I2caRegs.I2CDXR.bit.DATA = i2cBufferWriteA[i2cBufferIndexWriteA];
				i2cBufferIndexWriteA++;
		}
		// Ein NACK wurde empfangen.
		// Im Master-Receiver Mode nur möglich nach senden der Slave-Adresse
		else if (I2caRegs.I2CSTR.bit.NACK)
		{
				// NACK-Flag löschen
				I2caRegs.I2CSTR.bit.NACK = 1;
				// STOP-Bedingung senden
				I2caRegs.I2CMDR.bit.STP = 1;
				// Status-Flag setzen um ein Fehler in der Übertragung zu signalisieren
				i2cStatusFlagA = I2C_STATUS_ERROR;
		}
		// Im Non-Repeat Mode (RM in I2CMDR ist nicht gesetzt) wird ARDY gesetzt, sobald
		// die Anzahl von Bytes, die in I2CCNT steht, geschrieben bzw. gelesen wurde und keine
		// STOP-Bedingung abgesendet wurde (STP in I2CMDR wurde nach dem Start nicht gesetzt)
		// oder wenn ein NACK empfangen wurde. Damit letztgenannte Bedingung im Fehlerfall
		// (z.B. ungültige Slave-Adresse) nicht dazu führt, dass ständig eine wiederholte
		//  START-Bedingung getriggert wird, wird zusätzlich abgefragt, ob ein NACK empfangen
		// wurde
		if (I2caRegs.I2CSTR.bit.ARDY
				&& !I2caRegs.I2CSTR.bit.NACK)
		{
				// ARDY-Interrupt ausschalten (dieser wird nur einmalig
				// bei einer Schreib-Lese-Operation benötigt um das Ende
				// des Schreib-Datenpakets zu detektieren)
				I2caRegs.I2CIER.bit.ARDY = 0;
				// Master-Receiver Mode setzen
				I2caRegs.I2CMDR.bit.MST = 1;
				I2caRegs.I2CMDR.bit.TRX = 0;
				// Anzahl der zu lesenden Bytes setzen (Adress-Byte zählt nicht dazu)
				I2caRegs.I2CCNT = i2cBytesToReadAfterRSA;
				// Wiederholte START-Bedingung senden
				I2caRegs.I2CMDR.bit.STT = 1;
				// STOP-Bedingung senden, nachdem alle Bytes vom Slave gelesen wurden
				I2caRegs.I2CMDR.bit.STP = 1;
		}

		// Interrupt-Flag der Gruppe 8 löschen (da gehört der INT_I2CA-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK8 = 1;
}
