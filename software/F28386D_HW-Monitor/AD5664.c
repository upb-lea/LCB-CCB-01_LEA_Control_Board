//=================================================================================================
/// @file       AD5664.c
///
/// @brief      Datei enthält Variablen und Funktionen um den Digital-Analog-Converter AD5664
///							mithilfe der SPI-Libary "mySPI" zu steuern
///
/// @version    V1.0
///
/// @date       22.09.2021
///
/// @author     Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "AD5664.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Flag speichert den aktuellen Zustand der SPI-Kommunikation  (bereit/Kommunikation aktiv)
uint32_t ad5664StatusFlag = AD5664_STATUS_IN_PROGRESS;


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: AD5664Init ========================================================================
///
/// @brief  Funktion initialisiert das SPI-D und die GPIOs zur Kommunikation mit dem DAC. Der
///					Parameter "clock" gibt die gewünschte SPI-Taktfrequenz in Hz an. Die GPIOs werden
///					wie folgt konfiguiert:
///					MOSI: GPIO 91
///					MISO: GPIO 92
///					CLK : GPIO 93
///					SS  : GPIO 94
///
/// @param  void
///
/// @return uint32_t clock
///
//=================================================================================================
void AD5664Init(uint32_t clock)
{
    // Register-Schreibschutz aufheben
    EALLOW;

    // GPIO-Sperre für GPIO 91 (MOSI), 92 (MISO), 93 (CLK) und 94 (SS) aufheben
    GpioCtrlRegs.GPCLOCK.bit.GPIO91 = 0;
    GpioCtrlRegs.GPCLOCK.bit.GPIO92 = 0;
    GpioCtrlRegs.GPCLOCK.bit.GPIO93 = 0;
    GpioCtrlRegs.GPCLOCK.bit.GPIO94 = 0;
    // GPIO 92 auf SPI-Funktion setzen (MOSI)
    // Die Zahl in der obersten Zeile der Tabelle gibt den Wert für
    // GPAGMUX (MSB, 2 Bit) + GPAMUX (LSB, 2 Bit) als Dezimalzahl an
    // (siehe S. 1647 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPCGMUX2.bit.GPIO92 = (15 >> 2);
    GpioCtrlRegs.GPCMUX2.bit.GPIO92  = (15 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPCPUD.bit.GPIO92 = 1;
    // Asynchroner Eingang (muss für SPI gesetzt sein)
    GpioCtrlRegs.GPCQSEL2.bit.GPIO92 = 0x03;
    // GPIO 91 auf SPI-Funktion setzen (MISO)
    GpioCtrlRegs.GPCGMUX2.bit.GPIO91 = (15 >> 2);
    GpioCtrlRegs.GPCMUX2.bit.GPIO91  = (15 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPCPUD.bit.GPIO91 = 1;
    // Asynchroner Eingang (muss für SPI gesetzt sein)
    GpioCtrlRegs.GPCQSEL2.bit.GPIO91 = 0x03;
    // GPIO 93 auf SPI-Funktion setzen (CLK)
    GpioCtrlRegs.GPCGMUX2.bit.GPIO93 = (15 >> 2);
    GpioCtrlRegs.GPCMUX2.bit.GPIO93  = (15 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPCPUD.bit.GPIO93 = 1;
    // Asynchroner Eingang (muss für SPI gesetzt sein)
    GpioCtrlRegs.GPCQSEL2.bit.GPIO93 = 0x03;
    // GPIO 94 auf SPI-Funktion setzen (SS)
    GpioCtrlRegs.GPCGMUX2.bit.GPIO94 = (15 >> 2);
    GpioCtrlRegs.GPCMUX2.bit.GPIO94  = (15 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPCPUD.bit.GPIO94 = 1;
    // Asynchroner Eingang (muss für SPI gesetzt sein)
    GpioCtrlRegs.GPCQSEL2.bit.GPIO94 = 0x03;

    // Takt für das SPI-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
    CpuSysRegs.PCLKCR8.bit.SPI_D = 1;
    __asm(" RPT #4 || NOP");
		// SPI-Modul zum konfigurieren ausschalten
    SpidRegs.SPICCR.bit.SPISWRESET = 0;
    // Polarität = 0 (Ruhepegel: CLK = 0)
    SpidRegs.SPICCR.bit.CLKPOLARITY = 0;
    // Phase = 0 (Daten bei der ersten (mit POL = 0 also einer steigenden) Flanke übernehmen)
    SpidRegs.SPICTL.bit.CLK_PHASE = 0;
    // 8-Bit Datenlänge
    SpidRegs.SPICCR.bit.SPICHAR = 7;
    // Master-Mode setzen
    SpidRegs.SPICTL.bit.MASTER_SLAVE = 1;
    // Übertragung aktivieren
    SpidRegs.SPICTL.bit.TALK = 1;
    // Taktrate setzen
    // (Low-Speed CLK / clock) - 1
    // Low-Speed CLK = 50 MHz (siehe "DeviceInit()")
    SpidRegs.SPIBRR.bit.SPI_BIT_RATE = (50000000 / clock) - 1;
    // FIFO-Reset während der Konfiguration setzen
    SpidRegs.SPIFFTX.bit.TXFIFO = 0;
    // FIFO-Modus einschalten
    SpidRegs.SPIFFTX.bit.SPIFFENA = 1;
    // RX-FIFO Interrupt einschalten
    SpidRegs.SPIFFRX.bit.RXFFIENA = 1;
    // Interrupt auslösen, wenn 3 Bytes empfangen wurden
    SpidRegs.SPIFFRX.bit.RXFFIL = 3;
    // RX-FIFO Interupt-Flag löschen
    SpidRegs.SPIFFRX.bit.RXFFINTCLR = 1;
    // FIFO-Reset aufheben
    SpidRegs.SPIFFTX.bit.TXFIFO = 1;
    // SPI-Modul wieder einschalten
    SpidRegs.SPICCR.bit.SPISWRESET = 1;

    // CPU-Interrupts während der Konfiguration global sperren
    DINT;
    // Interrupt-Service-Routinen für den SPI-Interrupt an die
    // entsprechende Stelle (SPID_RX_INT) der PIE-Vector Table speichern
    PieVectTable.SPID_RX_INT = &AD5664SpiISR;
    // SPID_RX-Interrupt freischalten (Zeile 6, Spalte 11 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER6.bit.INTx11 = 1;
    // CPU-Interrupt 6 einschalten (Zeile 6 der Tabelle)
    IER |= M_INT6;
    // CPU-Interrupts nach Konfiguration global wieder freigeben
    EINT;

    // Status auf "bereit" setzen
    ad5664StatusFlag = AD5664_STATUS_IDLE;

		// Register-Schreibschutz setzen
		EDIS;
}


//=== Function: AD566SetChannel ===================================================================
///
/// @brief  Funktion setzt den Wert für einen Kanal (A, B, C oder D) des DAC. Der Wert wird
///					vom DAC anschließend direkt übernommen
///
/// @param  uint16_t channel, uint16_t value
///
/// @return void
///
//=================================================================================================
void AD566SetChannel(uint16_t channel,
										 uint16_t value)
{
		// Flag setzen um der aufrufenden Stelle zu signalisieren,
		// dass eine SPI-Kommunikation gestartet wurde
		ad5664StatusFlag = AD5664_STATUS_IN_PROGRESS;
		// Daten in den SPI-Hardware-Puffer kopieren (Daten
		// müssen links-bünig sein):
		// (Größe: siehe S. 3904 Reference Manual TMS320F2838x)
		// Steuer-Byte (neuen DAC-Wert sofort setzen)
		SpidRegs.SPITXBUF = ( (AD5664_WRITE_REG_SET_DAC | channel) << 8 );
		// Daten MSB
		SpidRegs.SPITXBUF = ( ((value >> 8) & 0xFF) << 8 );
		// Daten LSB
		SpidRegs.SPITXBUF = ( (value & 0xFF) << 8 );
}


//=== Function: AD5664SpiISR ======================================================================
///
/// @brief	Funktion wird aufgerufen, sobald drei Bytes über SPI empfangen wurde. Für den Betrieb
///					des Hardware-Monitors werden die Daten über SPI zwar nur gesendet, aber die Konfigu-
///					ration des Sende-Interrupts ist komplizierter als die des Emfangs-Interrupts. Zudem
///					wird der Sende-Interrupt ausgelöst, sobal der TX-Puffer leer ist. Zu diesem Zeitpunkt
///					befindet sich das letzte zu sendene Byte jedoch noch im Ausgnagsregister. Der Interrupt
///					kommt also zu früh. Aus diesem Grund, und weil beim Senden auch automatisch Daten
///					empfangen werden, wurde hier die Kommunikation mithilfe des Empfangs-Interrupts umge-
///					setzt.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void AD5664SpiISR(void)
{
		// Status auf "bereit" setzen
		ad5664StatusFlag = AD5664_STATUS_IDLE;
		// Empfangene Bytes auslesen
		uint16_t dummy = SpidRegs.SPIRXBUF;
		dummy = SpidRegs.SPIRXBUF;
		dummy = SpidRegs.SPIRXBUF;

    // RX-FIFO Interupt-Flag löschen
    SpidRegs.SPIFFRX.bit.RXFFINTCLR = 1;
		// Interrupt-Flag der Gruppe 6 löschen (da gehört der SPI-Interrupt zu)
    PieCtrlRegs.PIEACK.bit.ACK6 = 1;
}
