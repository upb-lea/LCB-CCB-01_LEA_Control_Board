//=================================================================================================
/// @file       myADC.c
///
/// @brief      Datei enthält Variablen und Funktionen um den internen Analog-Digital-Wandler
///							des TMS320F2838x zu nutzen. Der Code nutzt nur das ADC-A Modul. Die Nutzung
///							der Module B und C ist analog zu der des Moduls A. Der ADC wird so konfiguriert,
///							dass die externe 3,0 V-Referenz verwendet wird und der ADC mit 50 MHZ-Takt läuft
///							(SYSCLK = 200 MHz). Die Messung wird durch die Software in main() getriggert und
///						 	misst am Eingang ADCINA2/CMPIN1P.
///
/// @version    V1.2
///
/// @date       07.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myADC.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: AdcInitTrimRegister ===============================================================
///
/// @brief	Funktion läd die Exemplar-spezifischen Kalibrierwerte aus dem OTP-Speicher
///					und kopiert sie in die entsprechenden Register des ADCs
///
/// @param  uint32_t adcModule, uint32_t resolution, uint32_t signalMode
///
/// @return void
///
//=================================================================================================
extern void AdcInitTrimRegister(uint32_t adcModule,
																uint32_t resolution,
																uint32_t signalMode)
{
		// Trimm-Register für ADC-A initialisieren
		if (adcModule == ADC_MODULE_A)
		{
				// Pointer auf die Startadresse der Kalibrierungsdaten
		    // im OTP-Speicher des Mikrocontrollers
				uint32_t *ptr = ADC_A_INLTRIM_OTP_ADDR_START;
				// Wert für die 16 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
				uint16_t offsetTrim = *ADC_A_OFFSETTRIM_OTP_16BIT;
				// Liniearitäts-Trimmregister mit den (Exemplar-
				// spezifischen) Herstellungsdaten beschreiben
				AdcaRegs.ADCINLTRIM1 = *ptr;
				AdcaRegs.ADCINLTRIM2 = *(++ptr);
				AdcaRegs.ADCINLTRIM3 = *(++ptr);
				AdcaRegs.ADCINLTRIM4 = *(++ptr);
				AdcaRegs.ADCINLTRIM5 = *(++ptr);
				AdcaRegs.ADCINLTRIM6 = *(++ptr);
		    // Für den 12 Bit-Betrieb müssen einige Trim-Register maskiert
				// werden. Warum das gemacht werden muss und welche Register
				// das sind, steht nicht im Datenblatt sondern ist aus den
				// Beispielen der driverlib übernommen worden. Im Datenblatt
				// (S. 2521 Reference Manual TMS320F2838x) wird lediglich darauf
				// verwiesen, dass zur Konfiguration der Auflösung und Betriebsart
				// die TI-Funktion AdcSetMode() bzw. ADC_setMode() verwendet werden
				// solle und die Register nicht direkt beschrieben werden sollen
		    if(resolution == ADC_RESOLUTION_12_BIT)
		    {
		        AdcaRegs.ADCINLTRIM1 &= 0xFFFF0000;
		        AdcaRegs.ADCINLTRIM2 &= 0xFFFF0000;
		        AdcaRegs.ADCINLTRIM4 &= 0xFFFF0000;
		        AdcaRegs.ADCINLTRIM5 &= 0xFFFF0000;
		    		// Wert für die 12 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
		    		offsetTrim = *ADC_A_OFFSETTRIM_OTP_12BIT;
		    }
				// Wert für die Offset-Kalibrierung abhängig von der Betriebsart
				// setzen. Der Wert aus dem OTP-Speicher enthält die Kalibrierdaten
				// für beide Betriebsarten (MSB für Sigle-Ended, LSB für Differential)
				if (signalMode == ADC_SINGLE_ENDED_MODE)
				{
						AdcaRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim >> 8;
				}
				else
				{
						AdcaRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim & 0xFF;
				}
		}
		// Trimm-Register für ADC-B initialisieren
		else if (adcModule == ADC_MODULE_B)
		{
				// Pointer auf die Startadresse der Kalibrierungsdaten
		    // im OTP-Speicher des Mikrocontrollers
				uint32_t *ptr = ADC_B_INLTRIM_OTP_ADDR_START;
				// Wert für die 16 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
				uint16_t offsetTrim = *ADC_B_OFFSETTRIM_OTP_16BIT;
				// Liniearitäts-Trimmregister mit den (Exemplar-
				// spezifischen) Herstellungsdaten beschreiben
				AdcbRegs.ADCINLTRIM1 = *ptr;
				AdcbRegs.ADCINLTRIM2 = *(++ptr);
				AdcbRegs.ADCINLTRIM3 = *(++ptr);
				AdcbRegs.ADCINLTRIM4 = *(++ptr);
				AdcbRegs.ADCINLTRIM5 = *(++ptr);
				AdcbRegs.ADCINLTRIM6 = *(++ptr);
		    // Für den 12 Bit-Betrieb müssen einige Trim-Register maskiert
				// werden. Warum das gemacht werden muss und welche Register
				// das sind, steht nicht im Datenblatt sondern ist aus den
				// Beispielen der driverlib übernommen worden. Im Datenblatt
				// (S. 2521 Reference Manual TMS320F2838x) wird lediglich darauf
				// verwiesen, dass zur Konfiguration der Auflösung und Betriebsart
				// die TI-Funktion AdcSetMode() bzw. ADC_setMode() verwendet werden
				// solle und die Register nicht direkt beschrieben werden sollen
		    if(resolution == ADC_RESOLUTION_12_BIT)
		    {
		        AdcbRegs.ADCINLTRIM1 &= 0xFFFF0000;
		        AdcbRegs.ADCINLTRIM2 &= 0xFFFF0000;
		        AdcbRegs.ADCINLTRIM4 &= 0xFFFF0000;
		        AdcbRegs.ADCINLTRIM5 &= 0xFFFF0000;
		    		// Wert für die 12 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
		    		offsetTrim = *ADC_B_OFFSETTRIM_OTP_12BIT;
		    }
				// Wert für die Offset-Kalibrierung abhängig von der Betriebsart
				// setzen. Der Wert aus dem OTP-Speicher enthält die Kalibrierdaten
				// für beide Betriebsarten (MSB für Sigle-Ended, LSB für Differential)
				if (signalMode == ADC_SINGLE_ENDED_MODE)
				{
						AdcbRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim >> 8;
				}
				else
				{
						AdcbRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim & 0xFF;
				}
		}
		// Trimm-Register für ADC-C initialisieren
		else if (adcModule == ADC_MODULE_C)
		{
				// Pointer auf die Startadresse der Kalibrierungsdaten
		    // im OTP-Speicher des Mikrocontrollers
				uint32_t *ptr = ADC_C_INLTRIM_OTP_ADDR_START;
				// Wert für die 16 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
				uint16_t offsetTrim = *ADC_C_OFFSETTRIM_OTP_16BIT;
				// Liniearitäts-Trimmregister mit den (Exemplar-
				// spezifischen) Herstellungsdaten beschreiben
				AdccRegs.ADCINLTRIM1 = *ptr;
				AdccRegs.ADCINLTRIM2 = *(++ptr);
				AdccRegs.ADCINLTRIM3 = *(++ptr);
				AdccRegs.ADCINLTRIM4 = *(++ptr);
				AdccRegs.ADCINLTRIM5 = *(++ptr);
				AdccRegs.ADCINLTRIM6 = *(++ptr);
		    // Für den 12 Bit-Betrieb müssen einige Trim-Register maskiert
				// werden. Warum das gemacht werden muss und welche Register
				// das sind, steht nicht im Datenblatt sondern ist aus den
				// Beispielen der driverlib übernommen worden. Im Datenblatt
				// (S. 2521 Reference Manual TMS320F2838x) wird lediglich darauf
				// verwiesen, dass zur Konfiguration der Auflösung und Betriebsart
				// die TI-Funktion AdcSetMode() bzw. ADC_setMode() verwendet werden
				// solle und die Register nicht direkt beschrieben werden sollen
		    if(resolution == ADC_RESOLUTION_12_BIT)
		    {
		        AdccRegs.ADCINLTRIM1 &= 0xFFFF0000;
		        AdccRegs.ADCINLTRIM2 &= 0xFFFF0000;
		        AdccRegs.ADCINLTRIM4 &= 0xFFFF0000;
		        AdccRegs.ADCINLTRIM5 &= 0xFFFF0000;
		    		// Wert für die 12 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
		    		offsetTrim = *ADC_C_OFFSETTRIM_OTP_12BIT;
		    }
				// Wert für die Offset-Kalibrierung abhängig von der Betriebsart
				// setzen. Der Wert aus dem OTP-Speicher enthält die Kalibrierdaten
				// für beide Betriebsarten (MSB für Sigle-Ended, LSB für Differential)
				if (signalMode == ADC_SINGLE_ENDED_MODE)
				{
						AdccRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim >> 8;
				}
				else
				{
						AdccRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim & 0xFF;
				}
		}
		// Trimm-Register für ADC-D initialisieren
		else if (adcModule == ADC_MODULE_D)
		{
				// Pointer auf die Startadresse der Kalibrierungsdaten
		    // im OTP-Speicher des Mikrocontrollers
				uint32_t *ptr = ADC_D_INLTRIM_OTP_ADDR_START;
				// Wert für die 16 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
				uint16_t offsetTrim = *ADC_D_OFFSETTRIM_OTP_16BIT;
				// Liniearitäts-Trimmregister mit den (Exemplar-
				// spezifischen) Herstellungsdaten beschreiben
				AdcdRegs.ADCINLTRIM1 = *ptr;
				AdcdRegs.ADCINLTRIM2 = *(++ptr);
				AdcdRegs.ADCINLTRIM3 = *(++ptr);
				AdcdRegs.ADCINLTRIM4 = *(++ptr);
				AdcdRegs.ADCINLTRIM5 = *(++ptr);
				AdcdRegs.ADCINLTRIM6 = *(++ptr);
		    // Für den 12 Bit-Betrieb müssen einige Trim-Register maskiert
				// werden. Warum das gemacht werden muss und welche Register
				// das sind, steht nicht im Datenblatt sondern ist aus den
				// Beispielen der driverlib übernommen worden. Im Datenblatt
				// (S. 2521 Reference Manual TMS320F2838x) wird lediglich darauf
				// verwiesen, dass zur Konfiguration der Auflösung und Betriebsart
				// die TI-Funktion AdcSetMode() bzw. ADC_setMode() verwendet werden
				// solle und die Register nicht direkt beschrieben werden sollen
		    if(resolution == ADC_RESOLUTION_12_BIT)
		    {
		        AdcdRegs.ADCINLTRIM1 &= 0xFFFF0000;
		        AdcdRegs.ADCINLTRIM2 &= 0xFFFF0000;
		        AdcdRegs.ADCINLTRIM4 &= 0xFFFF0000;
		        AdcdRegs.ADCINLTRIM5 &= 0xFFFF0000;
		    		// Wert für die 12 Bit-Offset-Kalibrierung aus dem OTP-Speicher laden
		    		offsetTrim = *ADC_D_OFFSETTRIM_OTP_12BIT;
		    }
				// Wert für die Offset-Kalibrierung abhängig von der Betriebsart
				// setzen. Der Wert aus dem OTP-Speicher enthält die Kalibrierdaten
				// für beide Betriebsarten (MSB für Sigle-Ended, LSB für Differential)
				if (signalMode == ADC_SINGLE_ENDED_MODE)
				{
						AdcdRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim >> 8;
				}
				else
				{
						AdcdRegs.ADCOFFTRIM.bit.OFFTRIM = offsetTrim & 0xFF;
				}
		}
}


//=== Function: AdcAInit ==========================================================================
///
/// @brief	Funktion initialisiert den ADC (Modul A)
///
/// @param  uint32_t resolution, uint32_t signalMode
///
/// @return void
///
//=================================================================================================
void AdcAInit(uint32_t resolution,
							uint32_t signalMode)
{
		// Register-Schreibschutz aufheben
		EALLOW;

		// ADC konfigurieren:
    // Takt für das ADC-Modul einschalten und 5 Takte
    // warten, bis der Takt zum Modul durchgestellt ist
    // (siehe S. 169 Reference Manual TMS320F2838x)
		CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    __asm(" RPT #4 || NOP");
    // HINWEIS: Es gibt keine interne ADC-Referenzspannung!
    //					(siehe S. 2521 Reference Manual TMS320F2838x)
    //
    // Taktteiler auf 4 setzen => ADCCLK = SYSCLK / 4 = 50 MHz
    // maximaler ADCCLK = 50 MHz (siehe S. 138 Datasheet TMS320F2838x)
    AdcaRegs.ADCCTL2.bit.PRESCALE = ADC_CLK_DIV_4_0;
		// Stromversorgung des ADC einschalten
		AdcaRegs.ADCCTL1.bit.ADCPWDNZ = ADC_POWER_ON;
    // Erst 500 µs nach Einschalten des ADC kann
    // eine korrekte Messung durchgeführt werden
		// (siehe "Power Up Time" S. 139 + 142 Datasheet TMS320F2838x)
    DELAY_US(500);
    // Auflösung auf 12 Bit setzen
    // 0: 12 Bit-Auflösung (Achtung: nur Single-Ended möglich!)
    // 1: 16 Bit-Auflösung
    AdcaRegs.ADCCTL2.bit.RESOLUTION = resolution;
    // Single-Ended Mode als Betriebsart setzen
    // (siehe S. 2523 Reference Manual TMS320F2838x)
    // 0: Single-ended (Spannung an einem ADC-Pin gegen untere Referenzspannung
		//									bezogen auf Referenzsspannungsband: (ADCINx - VREFLO) / (VREFHI-VREFLO))
    // 1: Differential (Spannungsdifferenz an zwei ADC-Pins gegen 2*Referenzspannung:
    //									(ADCINxP - ADCINxN + VREFHI) / (2*VREFHI). In diesem Betriebsmodus
    //									muss VREFLO auf Analog-GND (VSSA) gelegt sein)
    AdcaRegs.ADCCTL2.bit.SIGNALMODE = signalMode;
    // Trimm-Register für Offset- und Linearitätsabweichung
    // mit Werten aus dem OTP-Speicher initialisieren
    AdcInitTrimRegister(ADC_MODULE_A,
												resolution,
												signalMode);


    // SOC konfigurieren:
    // Jede ADC-Messung wird durch sogenannte "Start Of Conversion" (SOC)
    // gesteuert. Jede SOC wird durch eine Trigger-Quelle (TRIGSEL), einen
    // Kanal für das Messsignal (CHSEL) und einem Abtastzeitraum (ACQPS)
    // definiert. Alle drei Parameter können unabhängig von anderen SOCs
    // gewählt werden. Falls zwei oder mehrere SOCs zum selben Zeitpunkt
    // getriggert werden, wird die Messung zu erst durchgeführt, die als
    // nächstes nach der aktuellen Position des Pointers auf dem "Round
    // Robin" steht (siehe S. 2532 Reference Manual TMS320F2838x). Falls
    // immer die selben SOCs und diese immer zum selben Zeitpunkt getriggert
    // werden sowie der Pointer vor dem ersten Triggern auf 16 steht (nach
    // einem Reset der Fall), wir die SOC mit der kleinsten Nummer zuerst
    // durchgeführt und anschließend die folgenden SOCs aufsteigend.
    // SOC0 konfigurieren:
    // Messung durch Software triggern.
    // Weitere Triggerquellen: CPU-Timer, ePWM-Module
    // (siehe S. 2607 Reference Manual TMS320F2838x)
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIGGER_SW_ONLY;
    // Eingang ADCINA2/CMPIN1P setzen
    // 0 : ADCIN0
    // 1 : ADCIN1
    // ...
    // 15: ADCIN15
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = ADC_SINGLE_ENDED_ADCIN2;
    // Abtastzeitfenster 60 (SYSCLK-)Taktzyklen = 300 ns.
    // ACQPS muss mindestens einen ADCLK-Takt lang sein.
    // Abtastzeitfenster = 1/SYSCLK * (ACQPS+1)
    // (siehe S. 2526 und 2556 Reference Manual + S. 145 Datasheet TMS320F2838x)
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 59;
    // Kein SOC-Trigger durch ADC-Interrupt.
    // Zusätzlich zu der in ADCSOC0CTL.TRIGSEL gesetzten Trigger-Quelle kann ein
    // ADC-Interrupt (ADCINT1 oder ADCINT2) wiederum selbst eine Messung triggern.
    // Somit kann ein kontinuierlicher Betrieb realisiert werden.
    // 0: Kein zusätzlicher Trigger durch ADCINT1/2
    // 1: ADCINT1 triggert eine Messung
    // 2: ADCINT2 triggert eine Messung
    // ADCINTSOCSEL1: SOC0 ... SOC7
    // ADCINTSOCSEL2: SOC8 ... SOC15
    AdcaRegs.ADCINTSOCSEL1.bit.SOC0 = ADC_NO_SOC_TRIGGER;

		// Register-Schreibschutz setzen
		EDIS;
}
