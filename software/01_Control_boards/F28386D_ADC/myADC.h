//=================================================================================================
/// @file       myADC.h
///
/// @brief      Datei enthält Variablen und Funktionen um den internen Analog-Digital-Wandler
///							des TMS320F2838x zu nutzen. Der Code nutzt nur das ADC-A Modul. Die Nutzung
///							der Module B und C ist analog zu der des Moduls A. Der ADC wird so konfiguriert,
///							dass die externe 3,0 V-Referenz verwendet wird und der ADC mit 50 MHz-Takt läuft
///							(SYSCLK = 200 MHz). Die Messung wird durch das ePWM8-Modul getriggert.
///						  Alternativ kann eine Messung auch "manuell" getriggert werden. Dazu ist in der
///							main() ein Stück Beispielcode gegeben. Nach dem Ende einer Messung wird ein
///							Interrupt ausgelöst und dort der Messwert in eine globale Variable kopiert.
///							Es werden beispielhaft drei Messungen (SOC) mit der selben Triggerquelle und
///							unterschiedlichen Eingängen/Kanälen konfiguriert.
///
/// @version    V1.2
///
/// @date       07.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
#ifndef MYADC_H_
#define MYADC_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"
#include "myPWM.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// ADC-Modul
#define ADC_MODULE_A												0
#define ADC_MODULE_B												1
#define ADC_MODULE_C												2
#define ADC_MODULE_D												3
// Taktteiler
#define ADC_CLK_DIV_1_0											0
#define ADC_CLK_DIV_2_0											2
#define ADC_CLK_DIV_2_5											3
#define ADC_CLK_DIV_3_0											4
#define ADC_CLK_DIV_3_5											5
#define ADC_CLK_DIV_4_0											6
#define ADC_CLK_DIV_4_5											7
#define ADC_CLK_DIV_5_0											8
#define ADC_CLK_DIV_5_5											9
#define ADC_CLK_DIV_6_0											10
#define ADC_CLK_DIV_6_5											11
#define ADC_CLK_DIV_7_0											12
#define ADC_CLK_DIV_7_5											13
#define ADC_CLK_DIV_8_0											14
#define ADC_CLK_DIV_8_5											15
// Energieversorgung
#define ADC_POWER_OFF												0
#define ADC_POWER_ON												1
// Auflösung
#define ADC_RESOLUTION_12_BIT								0
#define ADC_RESOLUTION_16_BIT								1
// Betriebsart
#define ADC_SINGLE_ENDED_MODE								0
#define ADC_DIFFERENTIAL_MODE								1
// Triggerquelle
#define ADC_TRIGGER_SW_ONLY			    	 			0
#define ADC_TRIGGER_CPU1_TIMER0 		 				1
#define ADC_TRIGGER_CPU1_TIMER1 		 				2
#define ADC_TRIGGER_CPU1_TIMER2					 		3
#define ADC_TRIGGER_GPIO    	   						4
#define ADC_TRIGGER_EPWM1_SOCA	 			 			5
#define ADC_TRIGGER_EPWM1_SOCB  						6
#define ADC_TRIGGER_EPWM2_SOCA   						7
#define ADC_TRIGGER_EPWM2_SOCB  						8
#define ADC_TRIGGER_EPWM3_SOCA  						9
#define ADC_TRIGGER_EPWM3_SOCB  						10
#define ADC_TRIGGER_EPWM4_SOCA  						11
#define ADC_TRIGGER_EPWM4_SOCB	  					12
#define ADC_TRIGGER_EPWM5_SOCA  						13
#define ADC_TRIGGER_EPWM5_SOCB  						14
#define ADC_TRIGGER_EPWM6_SOCA  						15
#define ADC_TRIGGER_EPWM6_SOCB  						16
#define ADC_TRIGGER_EPWM7_SOCA  						17
#define ADC_TRIGGER_EPWM7_SOCB  						18
#define ADC_TRIGGER_EPWM8_SOCA	  					19
#define ADC_TRIGGER_EPWM8_SOCB  						20
#define ADC_TRIGGER_EPWM9_SOCA  						21
#define ADC_TRIGGER_EPWM9_SOCB  						22
#define ADC_TRIGGER_EPWM10_SOCA  						23
#define ADC_TRIGGER_EPWM10_SOCB  						24
#define ADC_TRIGGER_EPWM11_SOCA  						25
#define ADC_TRIGGER_EPWM11_SOCB  						26
#define ADC_TRIGGER_EPWM12_SOCA  						27
#define ADC_TRIGGER_EPWM12_SOCB  						28
#define ADC_TRIGGER_CPU2_TIMER0  						29
#define ADC_TRIGGER_CPU2_TIMER1  						30
#define ADC_TRIGGER_CPU2_TIMER2						 	31
#define ADC_TRIGGER_EPWM13_SOCA	  					32
#define ADC_TRIGGER_EPWM13_SOCB  						33
#define ADC_TRIGGER_EPWM14_SOCA  						34
#define ADC_TRIGGER_EPWM14_SOCB  						35
#define ADC_TRIGGER_EPWM15_SOCA  						36
#define ADC_TRIGGER_EPWM15_SOCB  						37
#define ADC_TRIGGER_EPWM16_SOCA  						38
#define ADC_TRIGGER_EPWM16_SOCB  						39
// Eingangskanal
// Single-Ended Mode
#define ADC_SINGLE_ENDED_ADCIN0							0
#define ADC_SINGLE_ENDED_ADCIN1							1
#define ADC_SINGLE_ENDED_ADCIN2							2
#define ADC_SINGLE_ENDED_ADCIN3							3
#define ADC_SINGLE_ENDED_ADCIN4							4
#define ADC_SINGLE_ENDED_ADCIN5							5
#define ADC_SINGLE_ENDED_ADCIN6							6
#define ADC_SINGLE_ENDED_ADCIN7							7
#define ADC_SINGLE_ENDED_ADCIN8							8
#define ADC_SINGLE_ENDED_ADCIN9							9
#define ADC_SINGLE_ENDED_ADCIN10						10
#define ADC_SINGLE_ENDED_ADCIN11						11
#define ADC_SINGLE_ENDED_ADCIN12						12
#define ADC_SINGLE_ENDED_ADCIN13						13
#define ADC_SINGLE_ENDED_ADCIN14						14
#define ADC_SINGLE_ENDED_ADCIN15						15
// Differential Mode
#define ADC_DIFFERENTIAL_ADCIN0_ADCIN1			0
#define ADC_DIFFERENTIAL_ADCIN2_ADCIN3			2
#define ADC_DIFFERENTIAL_ADCIN4_ADCIN5			4
#define ADC_DIFFERENTIAL_ADCIN6_ADCIN7			6
#define ADC_DIFFERENTIAL_ADCIN8_ADCIN9			8
#define ADC_DIFFERENTIAL_ADCIN10_ADCIN11		10
#define ADC_DIFFERENTIAL_ADCIN12_ADCIN13		12
#define ADC_DIFFERENTIAL_ADCIN14_ADCIN15		14
// Start-of-Conversion
#define ADC_SOC_NUMBER_0										0
#define ADC_SOC_NUMBER_1										1
#define ADC_SOC_NUMBER_2										2
#define ADC_SOC_NUMBER_3										3
#define ADC_SOC_NUMBER_4										4
#define ADC_SOC_NUMBER_5										5
#define ADC_SOC_NUMBER_6										6
#define ADC_SOC_NUMBER_7										7
#define ADC_SOC_NUMBER_8										8
#define ADC_SOC_NUMBER_9										9
#define ADC_SOC_NUMBER_10										10
#define ADC_SOC_NUMBER_11										11
#define ADC_SOC_NUMBER_12										12
#define ADC_SOC_NUMBER_13										13
#define ADC_SOC_NUMBER_14										14
#define ADC_SOC_NUMBER_15										15
// End-of-Conversion
#define ADC_EOC_NUMBER_0										0
#define ADC_EOC_NUMBER_1										1
#define ADC_EOC_NUMBER_2										2
#define ADC_EOC_NUMBER_3										3
#define ADC_EOC_NUMBER_4										4
#define ADC_EOC_NUMBER_5										5
#define ADC_EOC_NUMBER_6										6
#define ADC_EOC_NUMBER_7										7
#define ADC_EOC_NUMBER_8										8
#define ADC_EOC_NUMBER_9										9
#define ADC_EOC_NUMBER_10										10
#define ADC_EOC_NUMBER_11										11
#define ADC_EOC_NUMBER_12										12
#define ADC_EOC_NUMBER_13										13
#define ADC_EOC_NUMBER_14										14
#define ADC_EOC_NUMBER_15										15
// SOC-Trigger durch ADC-Interrupt
#define ADC_NO_SOC_TRIGGER									0
#define ADC_ADCTIN1_TRIGGERS_SOC						1
#define ADC_ADCTIN2_TRIGGERS_SOC						2
// Triggerzeitpunkt
#define ADC_PULSE_END_OF_ACQ_WIN						0
#define ADC_PULSE_END_OF_CONV								1
// Interrupt ein-/ausschalten
#define ADC_INT_DISABLE											0
#define ADC_INT_ENABLE											1
// Interrupt-Impulserzeugung
#define ADC_INT_PULSE_ONE_SHOT							0
#define ADC_INT_PULSE_CONTINOUS							1


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
// Adressen im One-Time-Programmable Speicher (OTP, enthält u.a.
// Exemplar-spezifische Kalibrierungsdaten aus dem Herstellungsprozess)
// für die Offset-Kalibrierung der ADC-Module. Die Adressen wurden aus
// den Beispielprogrammen der driverlib entnommen (f2838x_example.h)
#define ADC_A_OFFSETTRIM_OTP_12BIT		((uint16_t *)0x70158)
#define ADC_A_OFFSETTRIM_OTP_16BIT		((uint16_t *)0x7015C)
#define ADC_B_OFFSETTRIM_OTP_12BIT		((uint16_t *)0x70159)
#define ADC_B_OFFSETTRIM_OTP_16BIT		((uint16_t *)0x7015D)
#define ADC_C_OFFSETTRIM_OTP_12BIT		((uint16_t *)0x7015A)
#define ADC_C_OFFSETTRIM_OTP_16BIT		((uint16_t *)0x7015E)
#define ADC_D_OFFSETTRIM_OTP_12BIT		((uint16_t *)0x7015B)
#define ADC_D_OFFSETTRIM_OTP_16BIT		((uint16_t *)0x7015F)
// Start-Adressen im One-Time-Programmable Speicher (OTP, enthält u.a.
// Exemplar-spezifische Kalibrierungsdaten aus dem Herstellungsprozess)
// für die Linearitäts-Kalibrierung der ADC-Module. Die Adressen wurden
// aus den Beispielprogrammen der driverlib entnommen (f2838x_example.h)
#define ADC_A_INLTRIM_OTP_ADDR_START	((uint32_t *)0x70128)
#define ADC_B_INLTRIM_OTP_ADDR_START	((uint32_t *)0x70134)
#define ADC_C_INLTRIM_OTP_ADDR_START	((uint32_t *)0x70140)
#define ADC_D_INLTRIM_OTP_ADDR_START	((uint32_t *)0x7014C)


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Messwerte der ADC-Eingänge ADCIN0, 1 und 2
extern uint16_t ADCIN0;
extern uint16_t ADCIN1;
extern uint16_t ADCIN2;


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion läd die Exemplar-spezifischen Kalibrierwerte
// aus dem OTP-Speicher und kopiert sie in die entsprechenden
// Register des ADCs
extern void AdcInitTrimRegister(uint32_t adcModule,
																uint32_t resolution,
																uint32_t signalMode);
// Funktion initialisiert den ADC (Modul A)
extern void AdcAInit(uint32_t resolution,
										 uint32_t signalMode);
// Interrupt-Service-Routine für den ADCINT1 (Modul A)
__interrupt void AdcAInt1ISR(void);


#endif




