//=================================================================================================
/// @file       myDevice.c
///
/// @brief      Datei enthält Funktionen um den Mikrocontorller TMS320F2838x grundlegend zu
///							initialisieren. Dazu wird der Watchdog-Timer ausgeschaltet, der Systemtakt
///							eingestellt, der Flash-Speicher initialisiert und die Interrupts freigeschaltet
///							und initialisiert.
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
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: DeviceInit ========================================================================
///
/// @brief  Funktion ruft abhänging von der ausführenden CPU die entsprechende
///					Initialisierungsfunktion auf
///
/// @param  uint32_t clockSource
///
/// @return void
///
//=================================================================================================
void DeviceInit(uint32_t clockSource)
{
#ifdef CPU1
		// Übergabeparameter für die Taktquelle prüfen
		if (   (clockSource == DEVICE_CLKSRC_INTOSC2)
				|| (clockSource == DEVICE_CLKSRC_EXTOSC_SE_25MHZ))
		{
				DeviceInitCPU1(clockSource);
		}
		// Falls kein gültiger Parameter übergeben wurde den
		// internen 10 MHZ-Oszillator verwenden
		else
		{
				DeviceInitCPU1(DEVICE_CLKSRC_INTOSC2);
		}
#else
		DeviceInitCPU2();
#endif
}


//=== Function: DeviceInitCPU1 ====================================================================
///
/// @brief  Funktion führt (durch CPU1) eine Grundinitialisation des Mikrocontrollers durch:
///					- Watchdog-Timer ausschalten
///					- Speicherinhalte zeitkritischer Funktionen von FLASH in dern RAM kopieren
///					- Flash-Speicher für 200 MHz initialisieren
///					- Systemtakt einstellen (interner 10 MHz-Oszillator oder externer (single-ended)
///																	 25 MHz Oszillator, 200 MHz Systemtakt, 50 MHz Low Speed CLK)
///					- Fabrikationsdaten aus dem OTP-Speicher in die ADC-Trimmregister kopieren
///					- CPU-Interrupts aus-, PIE-Vectrotabelle ein- und Interrupts global einschalten
///
/// @param  uint32_t clockSource
///
/// @return void
///
//=================================================================================================
void DeviceInitCPU1(uint32_t clockSource)
{
#ifdef CPU1
    // Watchdog-Timer ausschalten
    WdRegs.WDCR.bit.WDDIS = 1;

    // Flash-Speicher initialisieren:
    // Funktion zur Initialisierung des Flash-Speichers zur RAM-Sektion zuordnen
    // (wird über die .cmd-Datei durch den Linker entsprechen in den RAM kopiert)
    #pragma CODE_SECTION(DeviceInitFlashMemory, ".TI.ramfunc");
    // Zeitkritische Funktion in den RAM kopieren, wenn der Flash genutzt wird.
    // Wird das nicht gemacht, funktioniert der Code nicht weil z.B. die Funktion
    // DELAY_US() angehalten wird und das Programm dann nicht weiterläuft. Die
    // Namen der Variablen sind Platzhalter für Konstanten des Linkers und dürfen
		// daher nicht verändert werden.
    extern Uint16 RamfuncsRunStart, RamfuncsLoadStart, RamfuncsLoadSize;
#ifdef _FLASH
    // Flash-Initialisierungsfunktion in den RAM-Speicher kopieren
		memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
		// Flash initialisieren. Muss vom RAM aus aufgerufen werden
		DeviceInitFlashMemory();
#endif

    // Register-Schreibschutz aufheben
    EALLOW;

		// Interner 10 MHz Oszillator:
    if (clockSource == DEVICE_CLKSRC_INTOSC2)
    {
				// Siehe Reference Manual S. 173 TMS320F2838x:
				// PPL umgehen und 120 Takte warten
				// bis diese Änderung wirksam wird
				ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 0;
				asm(" RPT #119 || NOP");
				// PLL-Stromversorgung ausschalten und 60 Takte
				// warten bis diese Änderung wirksam wird
				ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 0;
				asm(" RPT #59 || NOP");
				// Internen Oszillator INTOSC2 (Primär-Oszillator) als Taktquelle
				// setzen und 300 Takte warten bis diese Änderung wirksam wird
				ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL = 0;
				// Wenn mehr als 255 wiederholungen in einem Befehl gesetzt
				// werden, kommt einen Warnung ([W0001] Value out of range).
				// Daher werden die 300 NOPs in 2 Befehle aufgeteilt
				asm(" RPT #200 || NOP");
				asm(" RPT #99 || NOP");
				// Taktteiler auf 1 setzen, damit die PLL schnellst
				// möglich kofiguriert werden kann
				ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 0;
				// Teiler und Miltiplikatoren der PLL konfigurieren
				// (siehe Reference Manual S. 172 TMS320F2838x):
				// f_PLL = (f_OSCCLK / (REFDIV+1)) * (IMULT / (ODIV+1))
				// f_PLL = 200 MHZ: REFDIV = 0; ODIV = 0, IMULT = 20
				// Achtung: REFDIV und IMULT müssen gleichzeitig gesetzt werden!
				// Andernfalls hängt sich der Microcontroller auf, siehe Reference
				// Manual S. 233 TMS320F2838x
				uint32_t REFDIV = 0;
				uint32_t IMULT  = 20;
				uint32_t ODIV   = 0;
				ClkCfgRegs.SYSPLLMULT.all = ((REFDIV << 24) | (ODIV << 16) | IMULT);
				// PLL-Stromversorgung einschalten
				ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 1;
				// Warten bis die PLL eingerastet ist
				while (ClkCfgRegs.SYSPLLSTS.bit.LOCKS == 0);
				// DCC-Modul initialisieren um den von der PLL
				// ausgegeben Takt zu überprüfen:
				// Takt für das DCC0-Modul einschalten
				CpuSysRegs.PCLKCR21.bit.DCC0 = 1;
				// Error- und Done-Flag löschen
				Dcc0Regs.DCCSTATUS.bit.ERR  = 1;
				Dcc0Regs.DCCSTATUS.bit.DONE = 1;
				// DCC0-Modul anhalten
				Dcc0Regs.DCCGCTRL.bit.DCCENA = 0x05;
				// Error- und Done-Interruptsignal ausschalten
				Dcc0Regs.DCCGCTRL.bit.ERRENA  = 0x05;
				Dcc0Regs.DCCGCTRL.bit.DONEENA = 0x05;
				// PLLRAWCLK als Messquelle
				Dcc0Regs.DCCCLKSRC1.all = 0xA000;
				// INTOSC2 als Referenzquelle
				Dcc0Regs.DCCCLKSRC0.all = 0xA002;
				// PLLRAWCLK überprüfen:
				// Frequenzverhältnis zwischen dem Messsignal und der Referenzquelle berechnen
				float ratio_fMeasure_fReference = (float)IMULT / ((ODIV + 1U) * (REFDIV + 1U));
				// Berechnung der Registerwerte nur für ratio_fMeasure_fReference >= 1 gültig!
				uint32_t toleranceInPercent, totalError, window, dccCounterSeed0, dccValidSeed0, dccCounterSeed1;
				// Gleichungen zur Berechnung: siehe Reference Manual S. 1319 TMS320F2838x
				toleranceInPercent = 1;
				totalError         = 12; // Wert aus DriverLib-Beispiel übernommen
				window             = (totalError * 100) / toleranceInPercent;
				dccCounterSeed0    = window - totalError;
				dccValidSeed0      = 2*totalError;
				dccCounterSeed1    = window * ratio_fMeasure_fReference;
				// Configure COUNTER-0, COUNTER-1 & Valid Window
				Dcc0Regs.DCCCNTSEED0.bit.COUNTSEED0  = dccCounterSeed0;
				Dcc0Regs.DCCVALIDSEED0.bit.VALIDSEED = dccValidSeed0;
				Dcc0Regs.DCCCNTSEED1.bit.COUNTSEED1  = dccCounterSeed1;
				// Single-Shot Betrieb des DCC-Moduls einschalten
				Dcc0Regs.DCCGCTRL.bit.SINGLESHOT = 0x0A;
				// DCC0-Modul starten
				Dcc0Regs.DCCGCTRL.bit.DCCENA = 0x0A;
				// Warten bis die Messung abgeschlossen ist
				while((Dcc0Regs.DCCSTATUS.all & 0x03) == 0);
				// Falls ein Fehler aufgetreten ist (Abweichung zwischen
				// Mess- und Refeenzsignal zu groß), Programm anhalten
				if ((Dcc0Regs.DCCSTATUS.all & 0x03) != 0x02)
				{
						__asm(" ESTOP0");
				}
				// Ab hier wird das Programm nur weiter ausgeführt,
				// falls die Takt-Initialisierung erfolgreich war.
				// Takteiler für für den Systemtakt um 1 größer setzen als der
				// tatsächliche Wert für den Betrieb um so die Stromaufnahme beim
				// umschalten der PLL als Systemtaktgeber zu reduzieren
				// (siehe Punkt 8 S. 173 Reference Manual TMS320F2838x)
				ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 1;
				// PLL als Systemtakt setzen und 100 Takte warten
				ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 1;
				asm(" RPT #199 || NOP");
				// Takteiler für für den Systemtakt auf 1 setzen (SYSCLK = PLLOUT)
				ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 0;
				// Taktteiler für den Low-Speed Peripheral Clock auf 4 setzen -> 50 MHz
				// Takt geht u.a. an: SPI- und UART-Clock
				// (siehe S. 165 Reference Manual TMS320F2838x)
				// 0: Teiler = 1
				// 1: Teiler = 2
				// 2: Teiler = 4
				// 3: Teiler = 6
				// 4: Teiler = 8
				// 5: Teiler = 10
				// 6: Teiler = 12
				// 7: Teiler = 14
				ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 2;
				// PWM-Vorteiler (SYSCLK -> EPWMCLK) auf 2 setzen
				// (siehe Reference Manual S. 2861 TMS320F2838x).
				// Dieser Takt geht auch zu den CLB-Modulen (siehe
				// Reference Manual S. 1176 TMS320F2838x)
				// 0: Teiler = 1
				// 1: Teiler = 2
				ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    }
    // Externer (Single-Ended) 25 MHz Oszillator
    else if (clockSource == DEVICE_CLKSRC_EXTOSC_SE_25MHZ)
    {
				// PPL umgehen und 120 Takte warten
				// bis diese Änderung wirksam wird
				ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 0;
				asm(" RPT #119 || NOP");
				// PLL-Stromversorgung ausschalten und 60 Takte
				// warten bis diese Änderung wirksam wird
				ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 0;
				asm(" RPT #59 || NOP");
				// Stromversorgung für Oszillator einschalten
				ClkCfgRegs.XTALCR.bit.OSCOFF = 0;
				// Betriebsart auf Single-Ended setzen
				ClkCfgRegs.XTALCR.bit.SE = 1;
				// Kurz warten bis der Oszillator eingeschaltet ist
				DELAY_US(1000);
				// Vier mal den Flankenzähler von Pin X1 zurücksetzen
				// (siehe Reference Manual S. 248 TMS320F2838x)
				for (uint32_t i=0; i<4; i++)
				{
						// Flankenzähler von Pin X1 zurücksetzen. Vorgang
						// solange wiederholen, bis der Zähler erfolgreich
						// zurückgesetzt wurde
						while (ClkCfgRegs.X1CNT.bit.X1CNT == 0x3FF)
						{
								// Zählerstand löschen
								ClkCfgRegs.X1CNT.bit.CLR = 1;
								// Löschvorgang beenden
								ClkCfgRegs.X1CNT.bit.CLR = 0;
						}
						// Warten bis der Endwert des Flankenzählers erreicht wurde
						while (ClkCfgRegs.X1CNT.bit.X1CNT < 0x3FF);
				}
				// Externen Oszillator als Taktquelle setzen
				ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL = 1;
				// Prüfen ob ein Takt besteht und Programm
				// anhalten, falls dies nicht der Fall ist
				// 0: Takt besteht
				// 1: Takt fehlt
				if(ClkCfgRegs.MCDCR.bit.MCLKSTS == 1)
				{
						__asm(" ESTOP0");
				}
				// Teiler und Miltiplikatoren der PLL konfigurieren
				// (siehe Reference Manual S. 172 TMS320F2838x):
				// f_PLL = (f_OSCCLK / (REFDIV+1)) * (IMULT / (ODIV+1))
				// f_PLL = 200 MHZ: REFDIV = 0; ODIV = 0, IMULT = 20
				// Achtung: REFDIV und IMULT müssen gleichzeitig gesetzt werden!
				// Andernfalls hängt sich der Microcontroller auf, siehe Reference
				// Manual S. 233 TMS320F2838x
				uint32_t REFDIV = 24;
				uint32_t IMULT  = 200;
				uint32_t ODIV   = 0;
				ClkCfgRegs.SYSPLLMULT.all = ((REFDIV << 24) | (ODIV << 16) | IMULT);
				// PLL-Stromversorgung einschalten
				ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 1;
				// Warten bis die PLL eingerastet ist
				while (ClkCfgRegs.SYSPLLSTS.bit.LOCKS == 0);
				// DCC-Modul initialisieren um den von der PLL
				// ausgegeben Takt zu überprüfen:
				// Takt für das DCC0-Modul einschalten
				CpuSysRegs.PCLKCR21.bit.DCC0 = 1;
				// Error- und Done-Flag löschen
				Dcc0Regs.DCCSTATUS.bit.ERR  = 1;
				Dcc0Regs.DCCSTATUS.bit.DONE = 1;
				// DCC0-Modul anhalten
				Dcc0Regs.DCCGCTRL.bit.DCCENA = 0x05;
				// Error- und Done-Interruptsignal ausschalten
				Dcc0Regs.DCCGCTRL.bit.ERRENA  = 0x05;
				Dcc0Regs.DCCGCTRL.bit.DONEENA = 0x05;
				// PLLRAWCLK als Messquelle
				Dcc0Regs.DCCCLKSRC1.all = 0xA000;
				// XTAL/X1 als Referenzquelle
				Dcc0Regs.DCCCLKSRC0.all = 0xA000;
				// PLLRAWCLK überprüfen:
				// Frequenzverhältnis zwischen dem Messsignal und der Referenzquelle berechnen
				float ratio_fMeasure_fReference = (float)IMULT / ((ODIV + 1U) * (REFDIV + 1U));
				// Berechnung der Registerwerte nur für ratio_fMeasure_fReference >= 1 gültig!
				uint32_t toleranceInPercent, totalError, window, dccCounterSeed0, dccValidSeed0, dccCounterSeed1;
				// Gleichungen zur Berechnung: siehe Reference Manual S. 1319 TMS320F2838x
				toleranceInPercent = 1;
				totalError         = 12; // Wert aus DriverLib-Beispiel übernommen
				window             = (totalError * 100) / toleranceInPercent;
				dccCounterSeed0    = window - totalError;
				dccValidSeed0      = 2*totalError;
				dccCounterSeed1    = window * ratio_fMeasure_fReference;
				// Configure COUNTER-0, COUNTER-1 & Valid Window
				Dcc0Regs.DCCCNTSEED0.bit.COUNTSEED0  = dccCounterSeed0;
				Dcc0Regs.DCCVALIDSEED0.bit.VALIDSEED = dccValidSeed0;
				Dcc0Regs.DCCCNTSEED1.bit.COUNTSEED1  = dccCounterSeed1;
				// Single-Shot Betrieb des DCC-Moduls einschalten
				Dcc0Regs.DCCGCTRL.bit.SINGLESHOT = 0x0A;
				// DCC0-Modul starten
				Dcc0Regs.DCCGCTRL.bit.DCCENA = 0x0A;
				// Warten bis die Messung abgeschlossen ist
				while((Dcc0Regs.DCCSTATUS.all & 0x03) == 0);
				// Falls ein Fehler aufgetreten ist (Abweichung zwischen
				// Mess- und Refeenzsignal zu groß), Programm anhalten
				if ((Dcc0Regs.DCCSTATUS.all & 0x03) != 0x02)
				{
						__asm(" ESTOP0");
				}
				// Ab hier wird das Programm nur weiter ausgeführt,
				// falls die Takt-Initialisierung erfolgreich war.
				// Takteiler für für den Systemtakt um 1 größer setzen als der
				// tatsächliche Wert für den Betrieb um so die Stromaufnahme beim
				// umschalten der PLL als Systemtaktgeber zu reduzieren
				// (siehe Punkt 8 Reference Manual S. 173 TMS320F2838x)
				ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 1;
				// PLL als Systemtakt setzen und 100 Takte warten
				ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 1;
				asm(" RPT #199 || NOP");
				// Takteiler für für den Systemtakt auf den Wert für den gewünschten Takt setzen
				ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 0;
				// Taktteiler für den Low-Speed Peripheral Clock auf 4 setzen -> 50 MHz
				// Takt geht u.a. an: SPI- und UART-Clock
				// (siehe S. 165 Reference Manual TMS320F2838x)
				// 0: Teiler = 1
				// 1: Teiler = 2
				// 2: Teiler = 4
				// 3: Teiler = 6
				// 4: Teiler = 8
				// 5: Teiler = 10
				// 6: Teiler = 12
				// 7: Teiler = 14
				ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 2;
				// PWM-Vorteiler (SYSCLK -> EPWMCLK) auf 2 setzen
				// (siehe Reference Manual S. 2861 TMS320F2838x).
				// Dieser Takt geht auch zu den CLB-Modulen (siehe
				// S. 1176 Reference Manual TMS320F2838x)
				// 0: Teiler = 1
				// 1: Teiler = 2
				ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    }
    // Ungültiger Funktionsparameter -> Programm abbrechen
    else
    {
    		__asm(" ESTOP0");
    }

    // Funktion kalibriert ADC-Referenz, DAC-Offset und die internen Oszillatoren
    DEVICE_CALIBRATION();

    // Interrupts initialisieren:
		// Register-Schreibschutz aufheben
		EALLOW;
		// Interrupts global ausschalten
		DINT;
		// Alle CPU-Interrupts ausschalten und deren Flags löschen
		IER = 0x0000;
		IFR = 0x0000;
		// PIE-Vector Table einschalten (speichert
		// die Adressen der Interupt-Service-Routinen)
		PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
		// Interrupts global einschalten
		EINT;

		// CPU2 booten
		DeviceBootCPU2();

		// Register-Schreibschutz setzen
		EDIS;
#endif
}


//=== Function: DeviceInitCPU2 ====================================================================
///
/// @brief  Funktion führt (durch CPU2) eine Initialisation durch:
///					- Speicherinhalte zeitkritischer Funktionen von FLASH in dern RAM kopieren
///					- CPU-Interrupts aus-, PIE-Vectrotabelle ein- und Interrupts global einschalten
///
/// @param  uint32_t clockSource
///
/// @return void
///
//=================================================================================================
void DeviceInitCPU2(void)
{
#ifdef CPU2
    // Zeitkritische Funktion in den RAM kopieren, wenn der Flash genutzt wird.
    // Wird das nicht gemacht, funktioniert der Code nicht weil z.B. die Funktion
    // DELAY_US() angehalten wird und das Programm dann nicht weiterläuft. Die
    // Namen der Variablen sind Platzhalter für Konstanten des Linkers und dürfen
		// daher nicht verändert werden.
    extern Uint16 RamfuncsRunStart, RamfuncsLoadStart, RamfuncsLoadSize;
#ifdef _FLASH
    // Flash-Initialisierungsfunktion in den RAM-Speicher kopieren
		memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

		// Register-Schreibschutz aufheben
		EALLOW;

    // Interrupts initialisieren:
		// Das Vorgehen entspricht exakt dem von CPU1 (gleiche Registernamen),
		// da CPU1 und CPU2 identisch aufgebaute PIE-Module besitzen und diese
		// entsprechend unabhängig von einander sind (siehe S. 146 Reference
		// Manual TMS320F2838x).
		// Interrupts global ausschalten
		DINT;
		// Alle CPU-Interrupts ausschalten und deren Flags löschen
		IER = 0x0000;
		IFR = 0x0000;
		// PIE-Vector Table einschalten (speichert
		// die Adressen der Interupt-Service-Routinen)
		PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
		// Interrupts global einschalten
		EINT;

		// Alle IPC-Flags löschen
		Cpu2toCpu1IpcRegs.CPU2TOCPU1IPCCLR.all = 0xFFFF;

		// Register-Schreibschutz setzen
		EDIS;
#endif
}


//=== Function: DeviceBootCPU2 ====================================================================
///
/// @brief  Funktion steuert den Boot-Prozess von CPU2
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void DeviceBootCPU2(void)
{
#ifdef CPU1
    // CPU2 durch CPU1 booten:
    // Register-Schreibschutz aufheben
    EALLOW;
    // Boot-Mode setzen (siehe S. 716 Reference Manual F2838x)
#ifdef _FLASH
    Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCBOOTMODE = (  DEVICE_CPU2_BOOTMODE_KEY
    																					 | DEVICE_CPU2_FREQ_200MHZ
																							 | DEVICE_CPU2_BOOTMODE_FLASH_SECTOR0);
#else
    Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCBOOTMODE = (  DEVICE_CPU2_BOOTMODE_KEY
    																					 | DEVICE_CPU2_FREQ_200MHZ
																							 | DEVICE_CPU2_BOOTMODE_RAM);
#endif
    // IPCFLG0 setzen (wird von CPU2 während ihres Boot-Prozesses
    // gelöscht, siehe S. 715 Reference Manual F2838x)
    Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCSET.bit.IPC0 = 1;
    // CPU2 aus dem Reset holen. Dieses Register wird nur dann verändert, wenn
    // das gesamte Register beschrieben wird (32 Bit Schreibbefehl) und dabei
    // die oberen 16 Bit mit einem gültigen Schlüssel beschrieben werden (siehe
    // S. 446 Reference Manual F2838x)
    // 0: CPU2-Reset deaktiviert
    // 1: CPU2-Reset aktiviert
    DevCfgRegs.CPU2RESCTL.all = (DEVICE_CPU2_RESET_KEY | DEVICE_CPU2_CLEAR_RESET);
    // Warten, bis CPU2 aus dem Reset ist
    // 0: CPU2 ist im Reset
    // 1: CPU2 ist nicht im Reset
    while (DevCfgRegs.RSTSTAT.bit.CPU2RES == DEVICE_CPU2_IS_IN_RESET);
    // Warten bis CPU2 mit dem Booten fertig ist (siehe S. 750 Reference Manual F2838x)
    while (Cpu1toCpu2IpcRegs.CPU2TOCPU1IPCBOOTSTS & DEVICE_CPU2_BOOTSTATE_FINISHED);
		// Alle IPC-Flags löschen
		Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCCLR.all = 0xFFFF;
#endif
}


//=== Function: DeviceInitFlashMemory =============================================================
///
/// @brief  Funktion initialisert den Flah-Speicher für einen CPU-Takt von 200 MHz
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void DeviceInitFlashMemory(void)
{
    // Register-Schreibschutz aufheben
    EALLOW;

    // Nach einem Reset sind Flash-Bank und -Pump
    // ausgeschaltet und müssen eingeschaltet werden
    Flash0CtrlRegs.FPAC1.bit.PMPPWR       = 0x01;
    Flash0CtrlRegs.FBFALLBACK.bit.BNKPWR0 = 0x03;
    // Cache und Prefetch vor dem Ändern der Wartezeit ausschalten
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 0;
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN   = 0;
    // Wartezeit für 200 MHz Systemfrequenz setzen
    // (Wert aus Beispielprogramm entnommen)
    Flash0CtrlRegs.FRDCNTL.bit.RWAIT = 0x03;
    // Cache und Prefetch nach dem Ändern der Wartezeit wieder
    // einschalten. Dadurch wird die Code-Performance verbessert
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 1;
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN   = 1;
    // Error Correction Code Protection ausschalten. Dieses Modul
    // kann fehler im Flash-Speicher erkennen und ausblenden
    // (siehe Reference Manual S. 1486 TMS320F2838x)
    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0x00;
    // 8 CPU-Takte warten damit die obigen Register-Operationen
    // abgeschlossen sind, bevor weiterer Code ausgeführt wird
    __asm(" RPT #7 || NOP");

		// Register-Schreibschutz setzen
		EDIS;
}
