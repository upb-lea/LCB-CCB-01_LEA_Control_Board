//=================================================================================================
/// @file       mySPI.h
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
#ifndef MYSPI_H_
#define MYSPI_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Größe der Software Sende- und Empfangs-Puffer für SPI-Kommunikation.
// Ausreichend groß und für den Duplexbetrieb gleich groß wählen
#define SPI_SIZE_BUFFER_TX								30
#define SPI_SIZE_BUFFER_RX								30
// Zustände der SPI-Kommunikation (spiStatusFlag)
#define SPI_STATUS_IDLE										0
#define SPI_STATUS_TX_IN_PROGRESS					3
#define SPI_STATUS_TX_FINISHED						4
#define SPI_STATUS_RX_IN_PROGRESS					1
#define SPI_STATUS_RX_FINISHED						2
#define SPI_STATUS_TX_RX_IN_PROGRESS 			5
#define SPI_STATUS_TX_RX_FINISHED    			6
// Paramter zur Auswahl des Slaves, mit dem kommuniziert werden soll
#define SPI_SELECT_SLAVE1									0x01
#define SPI_SELECT_SLAVE2									0x02
#define SPI_SELECT_SLAVE3									0x04
#define SPI_SELECT_SLAVE4									0x08
// Wert der an einen Slave während einer Lese-Operation gesendet wird
#define SPI_DUMMY_DATA										0x00
// Taktfrequenzen (CLK) für die SPI-Kommunikation
#define SPI_CLOCK_100_KHZ									100000
#define SPI_CLOCK_250_KHZ									250000
#define SPI_CLOCK_500_KHZ									500000
#define SPI_CLOCK_1_MHZ										1000000
#define SPI_CLOCK_2_MHZ										2000000
#define SPI_CLOCK_4_MHZ										4000000


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
// Gibt true zurück, wenn der entsprechende Slave angewählt ist (SS-Leitung auf logisch 0)
// Gibt false zurück, wenn der entsprechende Slave nicht angewählt ist (SS-Leitung auf logisch 1)
#define SPI_SLAVE1_IS_ENABLED							!((bool)GpioDataRegs.GPBDAT.bit.GPIO58)
#define SPI_SLAVE2_IS_ENABLED							!((bool)GpioDataRegs.GPBDAT.bit.GPIO59)
#define SPI_SLAVE3_IS_ENABLED							!((bool)GpioDataRegs.GPBDAT.bit.GPIO60)
#define SPI_SLAVE4_IS_ENABLED							!((bool)GpioDataRegs.GPBDAT.bit.GPIO61)
// Wählt den entsprechenden Slave an oder ab
#define SPI_ENABLE_SLAVE1									GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1
#define SPI_DISABLE_SLAVE1								GpioDataRegs.GPBSET.bit.GPIO58 = 1
#define SPI_ENABLE_SLAVE2									GpioDataRegs.GPBCLEAR.bit.GPIO59 = 1
#define SPI_DISABLE_SLAVE2								GpioDataRegs.GPBSET.bit.GPIO59 = 1
#define SPI_ENABLE_SLAVE3									GpioDataRegs.GPBCLEAR.bit.GPIO60 = 1
#define SPI_DISABLE_SLAVE3								GpioDataRegs.GPBSET.bit.GPIO60 = 1
#define SPI_ENABLE_SLAVE4									GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1
#define SPI_DISABLE_SLAVE4								GpioDataRegs.GPBSET.bit.GPIO61 = 1


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Software-Puffer für die SPI-Kommunikation
extern uint16_t spiBufferTxA[SPI_SIZE_BUFFER_TX];
extern uint16_t spiBufferRxA[SPI_SIZE_BUFFER_RX];


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion setzt GPIOs auf SPI-Funktionalität, initialisiert das SPI-A Modul
// als Master und aktiviert den SPI-Interrupt inkl. Registrierung der ISR
extern void SpiInitA(uint32_t clock);
// Funktion initialisiert den SPI Software Sende-Puffer (alle Elemente zu 0)
extern void SpiInitBufferTxA(void);
// Funktion initialisiert den SPI Software Empfangs-Puffer (alle Elemente zu 0)
extern void SpiInitBufferRxA(void);
// Funktion gibt den aktuellen Status der SPI-Kommunikation zurück
extern uint16_t SpiGetStatusA(void);
// Funktion setzt das Status-Flag auf "idle", falls die vorherige Kommunikation abgeschlossen ist
extern bool SpiSetStatusIdleA(void);
// Funktion sendet über SPI die mit dem Parameter "numberOfBytes" angegebene Anzahl an Bytes
// aus dem Software-Puffer "spiBufferTxA[]" an den über den Parameter "slaveSelect" adressierten
// Slave (Simplex-Transfer)
extern bool SpiWriteA(uint16_t slaveSelect,
										  uint16_t numberOfBytes);
// Funktion liest über SPI die mit dem Parameter "numberOfBytes" angegebene Anzahl an Bytes
// von dem über den Parameter "slaveSelect" adressierten Slave und speichert sie in den
// Software-Puffer "spiBufferRxA[]" (Simplex-Transfer)
extern bool SpiReadA(uint16_t slaveSelect,
										 uint16_t numberOfBytes);
// Funktion sendet über SPI die mit dem Parameter "numberOfBytes" angegebene Anzahl an Bytes
// aus dem Software-Puffer "spiBufferTxA[]" an den über den Parameter "slaveSelect" adressierten Slave,
// empfängt dieselbe Anzahl an Bytes von diesem und speichert sie in den Software-Puffer "spiBufferRxA[]"
// (Duplex-Transfer)
extern bool SpiWriteReadA(uint16_t slaveSelect,
												  uint16_t numberOfBytes);
// Funktion setzt den aktiven Slave inaktiv (setzen der SS-Leitung auf logisch 1)
extern void SpiDisableSlaveA(void);
// Interrupt-Service-Routine für die SPI-Kommunikation
__interrupt void SpiISRA(void);


#endif
