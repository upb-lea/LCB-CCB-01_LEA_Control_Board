//=================================================================================================
/// @file       myDevice.h
///
/// @brief      Datei enthält Funktionen um den Mikrocontorller TMS320F2838x grundlegend zu
///							initialisieren. Dazu wird der Watchdog-Timer ausgeschaltet, der Systemtakt
///							eingestellt, der Flash-Speicher initialisiert und die Interrupts freigeschaltet
///							und initialisiert.
///
/// @version    V1.1
///
/// @date       06.07.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
#ifndef MYDEVICE_H_
#define MYDEVICE_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
// Header für das CLA-Modul (muss vor "f2838x_device.h" eingebunden werden)
#include "f2838x_cla_typedefs.h"
// Header zur Nutzung der Registernamen und Einbindung von Standard-Bibliotheken
#include "f2838x_device.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Primärtaktquellen für den Systemtakt
// Keine Taktquelle (bei Aufruf durch CPU2)
#define DEVICE_DEFAULT													0
// Interner 10 MHz Oszillator
#define DEVICE_CLKSRC_INTOSC2										1
// Externer (Single-Ended) 25 MHz Oszillator
#define DEVICE_CLKSRC_EXTOSC_SE_25MHZ						2
// CPU2 Boot-Optionen
#define DEVICE_CPU2_BOOTMODE_KEY								0x5A000000UL
#define DEVICE_CPU2_FREQ_200MHZ									0xC800UL
#define DEVICE_CPU2_BOOTMODE_FLASH_SECTOR0			0x03
#define DEVICE_CPU2_BOOTMODE_FLASH_SECTOR4			0x23
#define DEVICE_CPU2_BOOTMODE_FLASH_SECTOR8			0x43
#define DEVICE_CPU2_BOOTMODE_FLASH_SECTOR13			0x63
#define DEVICE_CPU2_BOOTMODE_RAM								0x05
// CPU2 Boot-Status
#define DEVICE_CPU2_BOOTSTATE_FINISHED					0x80000000UL
// CPU2-Reset
#define DEVICE_CPU2_RESET_KEY										0xA5A50000UL
#define DEVICE_CPU2_CLEAR_RESET									0
#define DEVICE_CPU2_SET_RESET										1
#define DEVICE_CPU2_IS_NOT_IN_RESET							1
#define DEVICE_CPU2_IS_IN_RESET									0


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
// Delay-Funktion
// 200 MHz SYSCLK
#define DEVICE_CPU_RATE   											5.00L
// 190 MHz SYSCLK
//#define DEVICE_CPU_RATE   5.263L
// 180 MHz SYSCLK
//#define DEVICE_CPU_RATE   5.556L
// 170 MHz SYSCLK
//#define DEVICE_CPU_RATE   5.882L
// 160 MHz SYSCLK
//#define DEVICE_CPU_RATE   6.250L
// 150 MHz SYSCLK
//#define DEVICE_CPU_RATE   6.667L
// 140 MHz SYSCLK
//#define DEVICE_CPU_RATE   7.143L
// 130 MHz SYSCLK
//#define DEVICE_CPU_RATE   7.692L
// 120 MHz SYSCLK
//#define DEVICE_CPU_RATE   8.333L
extern void F28x_usDelay(long LoopCount);
#define DELAY_US(A)  														F28x_usDelay(((((long double) A * 1000.0L) / (long double)DEVICE_CPU_RATE) - 9.0L) / 5.0L)

// Makro zeigt auf Funktion, welche die ADC-Referenz, den DAC-Offset
// und die internen Oszillatoren kalibriert (direkt übernommen aus
// Beispielcode der Driverlib)
#define DEVICE_CALIBRATION ((void (*)(void))((uintptr_t)0x70260))


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion ruft abhänging von der ausführenden CPU
// die entsprechende Initialisierungsfunktion auf
extern void DeviceInit(uint32_t clockSource);
// Funktion führt eine Grundinitialisation des Mikrocontrollers durch
// durch (Watchdog-Timer, Systemtakt, Flash-Speicher, Interrupts) und
// kopiert bestimmte Speicherinhalte vom Flash in den RAM. Ausgeführt
// von CPU1
void DeviceInitCPU1(uint32_t clockSource);
// Funktion kopiert bestimmte Speicherinhalte vom Flash in den RAM
// und initialisiert die Interrupts. Ausgeführt von CPU2
void DeviceInitCPU2(void);
// Funktion steuert den Boot-Prozess von CPU2
void DeviceBootCPU2(void);
// Funktion initialisert den Flash-Speicher für 100 MHz Systemtakt
void DeviceInitFlashMemory(void);


#endif
