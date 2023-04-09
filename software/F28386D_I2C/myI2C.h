//=================================================================================================
/// @file       myI2C.h
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
#ifndef MYI2C_H_
#define MYI2C_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Anzahl Datenbits
#define I2C_DATA_BITS_1  					1
#define I2C_DATA_BITS_2  					2
#define I2C_DATA_BITS_3  					3
#define I2C_DATA_BITS_4 					4
#define I2C_DATA_BITS_5 					5
#define I2C_DATA_BITS_6  					6
#define I2C_DATA_BITS_7  					7
#define I2C_DATA_BITS_8  					0

// Größe der Software-Empfangs- und Sendepuffer für I2C-Kommunikation
#define I2C_SIZE_BUFFER_READ								30
#define I2C_SIZE_BUFFER_WRITE								30
// Zustände der I2C-Kommunikation (i2cStatusFlag)
#define I2C_STATUS_IDLE											0
#define I2C_STATUS_IN_PROGRESS							1
#define I2C_STATUS_FINISHED									2
#define I2C_STATUS_ERROR 										3
// Taktfrequenzen (SCL) für die I2C-Kommunikation
#define I2C_CLOCK_100_KHZ										1
#define I2C_CLOCK_400_KHZ										2


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Software-Puffer für die I2C-Kommunikation
extern uint16_t i2cBufferWriteA[I2C_SIZE_BUFFER_WRITE];
extern uint16_t i2cBufferReadA[I2C_SIZE_BUFFER_READ];


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion setzt GPIOs auf I2C-Funktionalität, initialisiert das I2C-A Modul
// als Master und aktiviert den I2C-Interrupt inkl. Registrierung der ISR
extern void I2cInitA(uint32_t clock);
// Funktion initialisiert den I2C Empfangs-Softwarepuffer (alle Elemente zu 0)
extern void I2cInitBufferReadA(void);
// Funktion initialisiert den I2C Empfangs-Softwarepuffer (alle Elemente zu 0)
extern void I2cInitBufferWriteA(void);
// Funktion gibt den aktuellen Status der I2C-Kommunikation zurück
extern uint16_t I2cGetStatusA(void);
// Funktion setzt das Status-Flag auf "idle", falls die vorherige Kommunikation abgeschlossen ist
extern bool I2cSetStatusIdleA(void);
// Funktion schreibt über I2C die mit dem Parameter "numberOfBytes" angegebene
// Anzahl an Bytes aus dem Software-Puffer "i2cBufferWrite[]" an den über den
// Parameter "slaveAddress" adressierten Slave (Master Transmitter Mode)
extern bool I2cWriteA(uint16_t slaveAddress,
										  uint16_t numberOfBytes);
// Funktion liest über I2C die mit dem Parameter "numberOfBytes" angegebene Anzahl
// an Bytes von dem über den Parameter "slaveAddress" adressierten Slave und speichert
// die Daten in dem Software-Puffer "i2cBufferRead[]" (Master Receiver Mode)
extern bool I2cReadA(uint16_t slaveAddress,
										 uint16_t numberOfBytes);
// Funktion schreibt über I2C die mit dem Parameter "numberOfBytesWrite" angegebene
// Anzahl an Bytes aus dem Software-Puffer "i2cBufferWrite[]" an den über den
// Parameter "slaveAddress" adressierten Slave (Master Transmitter Mode). Danach
// folgt eine wiederholte START-Bedingung und es wird die mit dem Parameter
// "numberOfBytesRead" angegebene Anzahl an Bytes vom selben Slave gelesen und
// in dem Software-Puffer "i2cBufferRead[]" gespeichert (Master Receiver Mode)
extern bool I2cWriteReadA(uint16_t slaveAddress,
													uint16_t numberOfBytesWrite,
													uint16_t numberOfBytesRead);
// Interrupt-Service-Routine für die I2C-Kommunikation
__interrupt void I2cISRA(void);


#endif
