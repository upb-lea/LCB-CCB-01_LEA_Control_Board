//=================================================================================================
/// @file       myGPIO.c
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
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myGPIO.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: GpioInit ==========================================================================
///
/// @brief  Funktion initialisiert GPIOs als Ausgänge
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void GpioInit(void)
{
		// Register-Schreibschutz aufheben
		EALLOW;

		// GPIO 3 (LED D1003 auf dem ControlBoard) als Ausgang:
		// Konfigurationssperre aufheben
		GpioCtrlRegs.GPALOCK.bit.GPIO3 = GPIO_CONFIG_UNLOCK;
		// Auf GPIO-Funktionalität setzen
		GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = (0 >> 2);
		GpioCtrlRegs.GPAMUX1.bit.GPIO3  = (0 & 0x03);
		// Pull-Up-Widerstand deaktivieren
		GpioCtrlRegs.GPAPUD.bit.GPIO3 = GPIO_DISABLE_PULLUP;
		// Auf Low-Pegel setzen
		GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
		// Als Ausgang setzen
		GpioCtrlRegs.GPADIR.bit.GPIO3 = GPIO_OUTPUT;

		// GPIO 5 (LED D1002 auf dem ControlBoard) als Ausgang:
		// Konfigurationssperre aufheben
		GpioCtrlRegs.GPALOCK.bit.GPIO5 = GPIO_CONFIG_UNLOCK;
		// Auf GPIO-Funktionalität setzen
		GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = (0 >> 2);
		GpioCtrlRegs.GPAMUX1.bit.GPIO5  = (0 & 0x03);
		// Pull-Up-Widerstand deaktivieren
		GpioCtrlRegs.GPAPUD.bit.GPIO5 = GPIO_DISABLE_PULLUP;
		// Auf Low-Pegel setzen
		GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;
		// Als Ausgang setzen
		GpioCtrlRegs.GPADIR.bit.GPIO5 = GPIO_OUTPUT;

		// GPIO 2 (LED D1004 auf dem ControlBoard) als Ausgang:
		// Konfigurationssperre aufheben
		GpioCtrlRegs.GPALOCK.bit.GPIO2 = GPIO_CONFIG_UNLOCK;
		// Auf GPIO-Funktionalität setzen
		GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = (0 >> 2);
		GpioCtrlRegs.GPAMUX1.bit.GPIO2  = (0 & 0x03);
		// Pull-Up-Widerstand deaktivieren
		GpioCtrlRegs.GPAPUD.bit.GPIO2 = GPIO_DISABLE_PULLUP;
		// Auf Low-Pegel setzen
		GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;
		// Als Ausgang setzen
		GpioCtrlRegs.GPADIR.bit.GPIO2 = GPIO_OUTPUT;
		// CPU2 steuert GPIO 2
		// 0: CPU1     steuert GPIO
		// 1: CPU1.CLA steuert GPIO
		// 2: CPU2     steuert GPIO
		// 3: CPU2.CLA steuert GPIO
		// 4: CM       steuert GPIO
		// ACHTUNG: CPU1 muss unabhängig von der Instanz, welche die GPIOs später steuern
		//					soll (CPU2, CLA, CM) die GPIOs vorher konfigurieren, da nur die Steuerung
		//				  über die GpioDataRegs-Register übertragen werden kann.
		GpioCtrlRegs.GPACSEL1.bit.GPIO2 = GPIO_CONTROLLED_BY_CPU2;

		// GPIO 6 (LED D1005 auf dem ControlBoard) als Ausgang:
		// Konfigurationssperre aufheben
		GpioCtrlRegs.GPALOCK.bit.GPIO6 = GPIO_CONFIG_UNLOCK;
		// Auf GPIO-Funktionalität setzen
		GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = (0 >> 2);
		GpioCtrlRegs.GPAMUX1.bit.GPIO6  = (0 & 0x03);
		// Pull-Up-Widerstand deaktivieren
		GpioCtrlRegs.GPAPUD.bit.GPIO6 = GPIO_DISABLE_PULLUP;
		// Auf Low-Pegel setzen
		GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;
		// Als Ausgang setzen
		GpioCtrlRegs.GPADIR.bit.GPIO6 = GPIO_OUTPUT;
		// CPU2 steuert GPIO 6
		// 0: CPU1     steuert GPIO
		// 1: CPU1.CLA steuert GPIO
		// 2: CPU2     steuert GPIO
		// 3: CPU2.CLA steuert GPIO
		// 4: CM       steuert GPIO
		// ACHTUNG: CPU1 muss unabhängig von der Instanz, welche die GPIOs später steuern
		//					soll (CPU2, CLA, CM) die GPIOs vorher konfigurieren, da nur die Steuerung
		//				  über die GpioDataRegs-Register übertragen werden kann.
		GpioCtrlRegs.GPACSEL1.bit.GPIO6 = GPIO_CONTROLLED_BY_CPU2;

    // Register-Schreibschutz setzen
    EDIS;
}




