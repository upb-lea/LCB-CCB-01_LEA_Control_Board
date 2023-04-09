//=================================================================================================
/// @file       AD5664_cpu1.c
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
#include "AD5664_cpu1.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: AD5664Init ========================================================================
///
/// @brief  Funktion initialisiert die GPIOs zur Kommunikation mit dem DAC. Anschließend wird
///					die Steuerung über das SPI-D-Modul an COU 2 übergeben. Die GPIOs werden wie folgt
///					konfiguiert:
///					MOSI: GPIO 91
///					MISO: GPIO 92
///					CLK : GPIO 93
///					SS  : GPIO 94
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void AD5664Init(void)
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

    // Steuerung über SPI-D an CPU 2 übergeben
    DevCfgRegs.CPUSEL6.bit.SPI_D = 1;
    // CPU 2 signalisieren, dass Sie nun die
    // Kontrolle über das SPI-D Modul hat
    Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCSET.bit.IPC0 = 1;


		// Register-Schreibschutz setzen
		EDIS;
}
