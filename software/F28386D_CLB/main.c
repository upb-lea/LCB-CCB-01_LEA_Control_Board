//=================================================================================================
/// @file			main.c
///
/// @brief		Diese Datei enthält das Hauptprogramm zur Demonstration des CLB-Moduls für den
///						Mikrocontroller TMS320F2838x. Die Konfiguration der CLB-Logik wird über die
///						Datei "emty.syscfg" vorgenommen, siehe dazu unten den Abschnitt "Grundlagen CLB".
///						Es wird die CLB1-Instanz verwendet. Als Logik wird ein UND-Gatter mit zwei Eingängen
///						und einem Ausgnag implementiert. Die beiden Eingänge sind GPIO 0 und 1, der Ausgang
///						ist GPIO 2. Hinweis: Dieses Projekt ist Driverlib-basiert, weil die .syscfg-Datei
///						Header- und Source-Dateien erzeugt, welche auf die Driverlib zugreift.
///
/// @version	V1.1
///
/// @date			08.09.2022
///
/// @author		Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"
#include "clb_config.h"


// Grundlagen CLB:
// https://www.ti.com/lit/ug/spruir8a/spruir8a.pdf
// https://www.ti.com/lit/an/spracl3/spracl3.pdf
//
//
// Das CLB-Modul (Configurable Logic Block = FPGA) besteht aus
// einem oder mehreren identischen (Teil-)Instanzen, s.g. Tiles.
//
// Der TMS320F28386/8 besitzt 8 CLB-Instanzen.
//
// Jedes CLB-Modul besteht aus verschieden Sub-Modulen:
// - LUT (Look Up Table)        : Wahrheitstabelle
// - FSM (Fenite State Machine) : Zustandsmaschine
// - Counter                    : Zähler
// - Output LUT                 : Ausgangs-Wahrheitstabelle (über diese werden die CLB-
//																                           Ausgangssignale aus der CLB-
//																													 Instanz herausgeführt)
// - HLC (High Level Controller): Event- und Interrupt-Logikblock
// - AOC (Asynchronous Output
//        Conditioning Block)   : Verarbeitung und Transport (zu SYSCLK) asynchroner Signale
//
// Jede CLB-Instanz besitzt 8 Eingänge und 8 Ausgänge. Die Ausgänge sind beim TMS320F2838x
// 4-fach repliziert, sodass insgesamt 32 Ausgänge zur Verfügung stehen (siehe S. 1190
// Reference Manual TMS320F2838x). Somit führen die Ausgänge OUT0, OUT8, OUT16 und OUT24
// dasselbe Signal.
//
// Auf die Eingänge können diverse Signale des Mikrocontrollers gelegt werden.
// Es gibt "Global Inputs" (Eingänge, die allen CLB-Instanzen zur Verfügung stehen)
// und "Local Inputs" (Eingänge, die für jede CLB-Instanz spezifisch sind).
//
// Die Ausgänge enthalten die verarbeiteten Informationen der CLB-Instanz und können
// über einen Demultiplexer auf diverse Peripherisignale gelegt werden. So kann z.B.
// ein PWM-Signal entweder vom ePWM-Modul stammen oder von einer CLB-Instanz. Welches
// von beiden Signalen an den GPIOs ausgegben wird, legt ein Multiplexer fest.
//
// Die Logik einer CLB-Instanz kann mithilfe eines CLB-Tools erstellt werden.
// Dieses Programm erzeugt eine .h- und eine .c-Datei, mit der die CLB-Register
// beschrieben werden um die gewünschten Logik-Funktionen abzubilden. Die Eingangs-
// und Ausgangsverbindungen der CLB-Instanzen werden nicht durch das Programm
// erstellt, sondern müssen manuell programmiert werden.
//
// Das CLB-Tool nutzt die Oberfläche des Programms "Sys-Config". Ggf. muss
// dieses noch installiert werden. Wichtig: Sys-Config muss zwingend unter
// "C:/ti/ccs<ccs-version>/ccs/utils/sysconfig_<syscfg-version>" installiert sein!
//
// Um das CLB-Tool zu öffnen, muss eine .syscfg-Datei vorliegen. Am einfachsten
// wird dazu über den Resource Explorer das Driverlib-basierte Projekt "clb_empty"
// importiert. In diesem Projekt befindet sich die Datei "empty.syscfg".
//
// Die CLB-Submodule (LUT, Counter, etc.) entsprechend der Anforderungen konfigurieren
// -> .syscfg-File speichern -> Dateien "clb_config.h" und "clb_config.c" werden im
// Projektpfad CPU1_RAM/syscfg bzw. CPU1_FLASH/syscfg nach dem Kompilieren gespeichert
// -> "clb_config.h" im Hauptprogramm inkludieren.
//
// Im Hauptprogramm zunächst den Takt für das verwendete CLB-Modul (1 ... 8) einschalten
// (PCLKCR17.bit.CLBx), die Initalisierungsfunktion aufrufen (Default-Name "initTILE1()",
// der tatsächliche Name kann der Datei "clb_config.h" entnommen werden), die Logik
// freischalten (ClbxLogicCtrlRegs.CLB_LOAD_EN.bit.GLOBAL_EN) und die Ein- und Ausgänge
// der CLB-Instanz konfigurieren.
//
// Die Eingangssignale werden der CLB-Instanz über die CLB X-Bar zugeführt. Alternativ
// können auch Ausgangssignale von anderen CLB-Intanzen als Eingangssignal gewählt werden.
//
// Die Ausgangssignale einer CLB-Instanz werden über die Output Look-Up-Tables
// herausgeführt und können über die verschiedenen X-Bars (ePWM, Output) oder
// dem "Peripheral Signal Multiplexer" auf interne Signale des Mikrocontrollers
// gelegt werden.


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Aus hw_memmap.h
#define CLB1_BASE                 0x00003000U
#define CLB2_BASE                 0x00003200U
#define CLB3_BASE                 0x00003400U
#define CLB4_BASE                 0x00003600U
#define CLB5_BASE                 0x00003800U
#define CLB6_BASE                 0x00003A00U
#define CLB7_BASE                 0x00003C00U
#define CLB8_BASE                 0x00003E00U


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//=== Function: main ==============================================================================
///
/// @brief  Hauptprogramm
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void main(void)
{
		// Mikrocontroller initialisieren (Watchdog, Systemtakt, Speicher, Interrupts)
		DeviceInit(DEVICE_CLKSRC_EXTOSC_SE_25MHZ);

		// Register-Schreibschutz aufheben
		EALLOW;
    // Takt für das CLB1-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 283 Reference Manual TMS320F2838x)
		CpuSysRegs.PCLKCR17.bit.CLB1 = 1;
    __asm(" RPT #4 || NOP");
    // CLB-Logik und -Register synchron zum Systemtakt betreiben
    // 0: CLB-Modul synchron zu SYSCLK
    // 1: CLB-Modul asynchron zu SYSCLK
    ClkCfgRegs.CLBCLKCTL.bit.CLKMODECLB1 = 0;
    /*
    // Die CLB-Clock darf max. 150 MHz betragen und über 100 MHz muss der PIPELINE-Modus
    // aktiviert werden (siehe S. 1176 Reference Manual TMS320F2838x). Da in der Funktion
    // "DeviceInit" der Vorteiler PERCLKDIVSEL.EPWMCLKDIV auf 2 gesetzt und SYSCLK 200 MHz
    // beträgt ist sichergestellt, dass die Anforderungen erfüllt sind.
    // Vorteiler auf 2 setzen
    // 0: Teiler = 1
    // 1: Teiler = 2
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    // Taktteiler für die CLB-Register
    ClkCfgRegs.CLBCLKCTL.bit.CLBCLKDIV = 0;
    // Taktteiler für die CLB-Logik
    // 0: Teiler = 1
    // 1: Teiler = 2
    // ...
    // 8: Teiler = 8
    ClkCfgRegs.CLBCLKCTL.bit.TILECLKDIV = 0;
    */
    // CLB-Block initialisieren
    initTILE1(CLB1_BASE);
    // Nach dem Funktionsaufruf muss der
    // Schreibschutz erneut aufgehoben werden
    EALLOW;
    // CLB-Modul einschalten
    Clb1LogicCtrlRegs.CLB_LOAD_EN.bit.GLOBAL_EN = 1;
    // Alle CLB_LOGIC_CONFIG_REGS- und einige der
    // CLB_LOGIC_CONTROL_REGS-Register können nur
    // mit deaktiviertem Schreibschutz bearbeitet
    // werden (Clb1LogicCtrlRegs.CLB_LOCK.bit.LOCK = 0).
    // Das Bit kann nur gesetzt werden und das nur einmalig.
    // Gelöscht wird es durch einen Reset (siehe S. 1279
    // Reference Manual TMS320F2838x)


    //*****************************************************
    // CLB-Eingänge konfigurieren:
    //*****************************************************
    // Eingang 0 / BOUNDARY IN0 (erstes Eingangssignal) konfigurieren:
    // Signal AUXSIG0 (CLB X-Bar) als Eingangssignal des globalen Eingangs
    // setzen (siehe Figure 9-5 und Table 9-2 S. 1178 ff. Reference Manual
    // TMS320F2838x)
    // GLBL_MUX_SEL_IN_0: Eingang 0
    // ...
    // GLBL_MUX_SEL_IN_7: Eingang 7
    Clb1LogicCtrlRegs.CLB_GLBL_MUX_SEL_1.bit.GLBL_MUX_SEL_IN_0 = 64;
    // Globalen Eingang 0 verwenden
    // 0: Globalen Eingang
    // x: Lokalen Eingang
    // LCL_MUX_SEL_IN_0: Eingang 0
    // ...
    // LCL_MUX_SEL_IN_7: Eingang 7
    Clb1LogicCtrlRegs.CLB_LCL_MUX_SEL_1.bit.LCL_MUX_SEL_IN_0 = 0;
    // Signalsynchronisation einschalten.
    // Muss für alle Signale eingeschaltet werden, die nach den Tabellen
    // "Table 9-2 Global Signals and Mux Selection" und "Table 9-3 Local
    // Signals and Mux Selection" mit "ASYNC" markiert sind (siehe S. 1179
    // ff. bzw. S. 1185 ff. Reference Manual TMS320F2838x)
    // 0: Synchronisation ausgeschaltet
    // 1: Synchronisation eingeschaltet
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.SYNC0 = 1;
    // Keine Signalfilterung verwenden
    // 0: Keine Filterung
    // 1: Steigende Flanken durchlassen
    // 2: Fallende Flanken durchlassen
    // 4: Beide Flanken durchlassen
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.FIN0 = 0;
    // Das zuvor konfigurierte (externe) Signal verwenden
    // 0: BOUNDARY INx ist externes Signal
    // 1: BOUNDARY INx ist GPREGx
    Clb1LogicCtrlRegs.CLB_IN_MUX_SEL_0.bit.SEL_GP_IN_0 = 0;
    // Eingang 1 / BOUNDARY IN1 (zweites Eingangssignal) konfigurieren:
    // Signal AUXSIG1 (CLB X-Bar) als globalen Eingang setzen
    Clb1LogicCtrlRegs.CLB_GLBL_MUX_SEL_1.bit.GLBL_MUX_SEL_IN_1 = 65;
    // Globalen Eingang 1 verwenden
    Clb1LogicCtrlRegs.CLB_LCL_MUX_SEL_1.bit.LCL_MUX_SEL_IN_1 = 0;
    // Signalsynchronisation einschalten
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.SYNC1 = 1;
    // Keine Signalfilterung verwenden
    Clb1LogicCtrlRegs.CLB_INPUT_FILTER.bit.FIN1 = 0;
    // Das zuvor konfigurierte (externe) Signal verwenden
    Clb1LogicCtrlRegs.CLB_IN_MUX_SEL_0.bit.SEL_GP_IN_1 = 0;


		//*****************************************************
		// Eingangs-GPIO konfigurieren:
		//*****************************************************
		// GPIO 0 und GPIO 1 sind die Eingänge der CLB-Instanz.
    // Konfigurationssperre aufheben
    GpioCtrlRegs.GPALOCK.bit.GPIO0 = 0;
    GpioCtrlRegs.GPALOCK.bit.GPIO1 = 0;
    // Auf GPIO-Funktionalität setzen
    // (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = (0x00 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO0  = (0x00 & 0x03);
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = (0x00 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO1  = (0x00 & 0x03);
    // Pull-Up-Widerstand aktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0;
    // Samplingperiode auf 2*SYSCLK setzen
    GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = 1;
    // GPIOs mit jedem Sampleimpuls einlesen
    GpioCtrlRegs.GPAQSEL1.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0;
    // Als Eingang setzen
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 0;


    //*****************************************************
    // Input X-Bar konfigurieren:
    //*****************************************************
    // Die Input X-Bar hat 16 Ausgänge, auf die jeweils einer der
    // GPIOs des Mikrocontrollers gelegt werden kann. Die Signale
    // der GPIOs können nicht invertiert werden, wie beispielsweise
    // bei der CLB oder ePWM X-Bar.
    // GPIO 0 auf Input1 legen
    // 0 = GPIO 0
    // ...
    // n = GPIO n
    InputXbarRegs.INPUT1SELECT = 0;
    // GPIO 1 auf Input2 legen
    InputXbarRegs.INPUT2SELECT = 1;


    //*****************************************************
    // CLB X-Bar konfigurieren:
    //*****************************************************
    // Die CLB X-Bar hat 8 Ausgänge (AUXSIG0 bis AUXSIG7), die jeweils
    // bis zu 32 Signale verodern. Jeder der 32 Multiplexer (MUX0 ...
    // MUX31) kann aus bis zu 4 Signalen wählen. Je nach gewünschtem
    // Signal muss der entsprechende Multiplexer ausgewählt werden. An-
    // schließend muss das Ausgangssignal des Multiplexers noch durchge-
    // schleift werden. Optional kann das aus den 32 Signalquellen vero-
    // derte Gesamtsignal noch invertiert werden (siehe S. 2146  ff.
    // Reference Manual TMS320F2838x). Die erste Spalte der Tabelle 17-4
    // gibt an, welcher Multiplexer (0 ... 15) für das gewünschte Signal
    // verwendet werden muss. Dabei ist darauf zu achten, dass jedes CLB-Signal
    // (AUXSIGn) einen entsprechenden Multiplexer hat. Die oberste Zeile
    // der Tabelle gibt den Registerwert für den entsprechenden Multiplexer an.
    // INPUT1 der Input X-Bar auf den AUXSIG0-Ausgang legen
    CLBXbarRegs.AUXSIG0MUX0TO15CFG.bit.MUX1 = 1;
    // MUX1 (INPUT1-Signal) durchschalten
    // 0: Signal gesperrt
    // 1: Signal durchgeschaltet
    CLBXbarRegs.AUXSIG0MUXENABLE.bit.MUX1 = 1;
    // Signal nicht invertieren
    // 0: Signal nicht invertieren
    // 1: Signal invertieren
    CLBXbarRegs.AUXSIGOUTINV.bit.OUT0 = 0;
    // INPUT2 der Input X-Bar auf den AUXSIG1-Ausgang legen
    CLBXbarRegs.AUXSIG1MUX0TO15CFG.bit.MUX3 = 1;
    // MUX3 (INPUT2-Signal) durchschalten
    CLBXbarRegs.AUXSIG1MUXENABLE.bit.MUX3 = 1;
    // Signal nicht invertieren
    CLBXbarRegs.AUXSIGOUTINV.bit.OUT1 = 0;


    //*****************************************************
    // CLB-Ausgänge konfigurieren:
    //*****************************************************
    // Falls Peripherie-Signale übersteuert werden solllen, muss das entsprechende
    // Bit im Multiplexer gesetzt werden. Je nach dem, welches Peripherie-Signal
    // beeinflusst werden soll, muss die entsprechende Output Look-Up-Table ausge-
    // wählt und der Multiplexer gesetzt werden (siehe S. 1191 ff. Reference Manual
    // TMS320F2838x).
    // Ausgang 0 (Output LUT 0) auf das ePWM1A-Signal legen
    //Clb1LogicCtrlRegs.CLB_OUT_EN = 1;


    //*****************************************************
    // Output X-Bar konfigurieren:
    //*****************************************************
    // Die Output X-Bar hat 8 Ausgänge (OUTPUT1 bis OUTPUT8), die jeweils
    // bis zu 32 Signale führen können. Jeder der 32 Multiplexer (MUX0
    // ... MUX31) kann aus bis zu 4 Signalen wählen. Je nach gewünschtem
    // Signal muss der entsprechende Multiplexer ausgewählt werden. An-
    // schließend muss das Ausgangssignal des Multiplexers noch durchge-
    // schleift werden. Optional kann das aus den 32 Signalquellen vero-
    // derte Gesamtsignal noch über ein latch geführt sowie invertiert
    // werden. Siehe S. 2148  ff. Reference Manual TMS320F2838x. Die erste
    // Spalte der Tabelle 17-5 gibt an, welcher Multiplexer für das gewünschte
    // Signal verwendet werden muss.
    // Nach Tabelle 15-7 (S. 1645 Reference Manual TMS320F2838x) kann auf GPIO 2
    // nur der Ausgang 1 der Output X-Bar gelegt werden. Daher wird dieser im
    // Folgenden konfiguriert.
    // CLB1_OUT4 auf X-Bar Output 1 legen
    OutputXbarRegs.OUTPUT1MUX0TO15CFG.bit.MUX1 = 2;
    // MUX1 durchschalten
    OutputXbarRegs.OUTPUT1MUXENABLE.bit.MUX1 = 1;
    // Latch umgehen
    OutputXbarRegs.OUTPUTLATCHENABLE.bit.OUTPUT1 = 0;
    // Signal nicht invertieren
		OutputXbarRegs.OUTPUTINV.bit.OUTPUT1 = 0;


		//*****************************************************
		// Ausgangs-GPIO konfigurieren:
		//*****************************************************
    // GPIO 2 ist der Ausgang der CLB-Instanz.
    // Konfigurationssperre aufheben
		// (siehe S. 1645 Reference Manual TMS320F2838x)
    GpioCtrlRegs.GPALOCK.bit.GPIO2 = 0;
    // Auf Output X-Bar Funktionalität setzen (OUTPUTXBAR3)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = (0x05 >> 2);
    GpioCtrlRegs.GPAMUX1.bit.GPIO2  = (0x05 & 0x03);
    // Pull-Up-Widerstand deaktivieren
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;
    // Als Ausgang setzen
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;


		// Dauerschleife Hauptprogramm
    while(1)
    {

    }
}

