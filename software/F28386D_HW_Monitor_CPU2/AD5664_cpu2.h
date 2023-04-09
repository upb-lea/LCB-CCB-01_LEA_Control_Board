//=================================================================================================
/// @file       AD5664_cpu2.h
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
#ifndef AD5664_CPU2_H_
#define AD5664_CPU2_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// SPI-Kommunikation:
// SPI-Clock
#define AD5664_SPI_CLOCK_1MHZ						1000000
#define AD5664_SPI_CLOCK_2MHZ						2000000
#define AD5664_SPI_CLOCK_4MHZ						4000000
#define AD5664_SPI_CLOCK_8MHZ						8000000
#define AD5664_SPI_CLOCK_12MHZ					12000000
#define AD5664_SPI_CLOCK_16MHZ					16000000
#define AD5664_SPI_CLOCK_25MHZ					25000000
// Betriebszustände der SPI-Kommunikation
#define AD5664_STATUS_IDLE							0
#define AD5664_STATUS_IN_PROGRESS				1

// DAC-Protokoll:
// Befehl/Steuerung
#define AD5664_WRITE_REG								0x00
#define AD5664_SET_DAC									0x08
#define AD5664_WRITE_REG_SET_ALL				0x10
#define AD5664_WRITE_REG_SET_DAC				0x18
#define AD5664_POWER_DOWN								0x20
#define AD5664_RESET										0x28
#define AD5664_SET_LATCH_MODE						0x30
// Kanal-Auswahl
#define AD5664_CHANNEL_A								0x00
#define AD5664_CHANNEL_B								0x01
#define AD5664_CHANNEL_C								0x02
#define AD5664_CHANNEL_D								0x03


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Flag speichert den aktuellen Zustand der SPI-Kommunikation (bereit/Kommunikation aktiv)
extern uint32_t ad5664StatusFlag;


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion initialisiert das SPI-D und die GPIOs zur Kommunikation mit dem DAC
extern void AD5664Init(uint32_t clock);
// Funktion sendet einen Wert für einen Kanal des DAC
extern void AD566SetChannel(uint16_t channel,
														uint16_t value);
// SPI-Interrupt-Routine zur Kommunikation mit dem DAC
__interrupt void AD5664SpiISR(void);


#endif

