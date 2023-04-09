//=================================================================================================
/// @file       myGPIO.h
///
/// @brief      Datei enthält Variablen und Funktionen um die GPIOs des Mikrocontrollers
///						  TMS320F2838x als Ausgänge, Eingänge oder Eingänge für externe interrupts
///							zu konfigurieren.
///
/// @version    V1.1
///
/// @date       05.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
#ifndef MYGPIO_H_
#define MYGPIO_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Multiplexer GPIO-Funktionalität
#define GPIO_MULTIPLEX_GPIO										0
// Konfigurationssperre
#define GPIO_CONFIG_UNLOCK										0
#define GPIO_CONFIG_LOCK											1
// Pullup-Widerstand
#define GPIO_ENABLE_PULLUP										0
#define GPIO_DISABLE_PULLUP										1
// Datenrichtung
#define GPIO_INPUT														0
#define GPIO_OUTPUT														1
// Steuernde Instanz
#define GPIO_CONTROLLED_BY_CPU1								0
#define GPIO_CONTROLLED_BY_CLA_CPU1						1
#define GPIO_CONTROLLED_BY_CPU2 							2
#define GPIO_CONTROLLED_BY_CLA_CPU2						3
#define GPIO_CONTROLLED_BY_CM									4


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion initialisiert GPIOs als Ausgänge
extern void GpioInit(void);


#endif

