//=================================================================================================
/// @file       myPWM.h
///
/// @brief      Datei enthält Variablen und Funktionen um die Funktion der ePWM-Module zu
///							demonstrieren. Die ePWM-Module 1 bis 3 werden so initialisiert, dass damit
///							ein 3-phasiger Wechselrichter angesteuert werden kann. Das ePWM8-Modul wird
///							so initialisiert, dass alle 100 ms ein Interrupt ausgelöst wird und so als
///							Zeitgeber für periodisch zu erledigende Aufgaben genutzt werden kann.
///
/// @version    V1.1
///
/// @date       06.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
#ifndef MYPWM_H_
#define MYPWM_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Taktteiler
#define PWM_CLK_DIV_1   										0
#define PWM_CLK_DIV_2												1
#define PWM_CLK_DIV_4												2
#define PWM_CLK_DIV_8												3
#define PWM_CLK_DIV_16											4
#define PWM_CLK_DIV_32											5
#define PWM_CLK_DIV_64											6
#define PWM_CLK_DIV_128											7
#define PWM_HSPCLKDIV_1  										0
#define PWM_HSPCLKDIV_2  										1
#define PWM_HSPCLKDIV_4  										2
#define PWM_HSPCLKDIV_6  										3
#define PWM_HSPCLKDIV_8  										4
#define PWM_HSPCLKDIV_10 										5
#define PWM_HSPCLKDIV_12  									6
#define PWM_HSPCLKDIV_14  									7
// Phasenversatz
#define PWM_TB_PHSEN_DISABLE  							0
#define PWM_TB_PHSEN_ENABLE  								1
// Synchronisationseingang
#define PWM_TB_SYNCIN_DISABLED       				0
#define PWM_TB_SYNCIN_EPWM1_SYNCOUT      		1
#define PWM_TB_SYNCIN_EPWM2_SYNCOUT      		2
#define PWM_TB_SYNCIN_EPWM3_SYNCOUT      		3
#define PWM_TB_SYNCIN_EPWM4_SYNCOUT      		4
#define PWM_TB_SYNCIN_EPWM5_SYNCOUT      		5
#define PWM_TB_SYNCIN_EPWM6_SYNCOUT      		6
#define PWM_TB_SYNCIN_EPWM7_SYNCOUT      		7
#define PWM_TB_SYNCIN_EPWM8_SYNCOUT      		8
#define PWM_TB_SYNCIN_EPWM9_SYNCOUT      		9
#define PWM_TB_SYNCIN_EPWM10_SYNCOUT      	10
#define PWM_TB_SYNCIN_EPWM11_SYNCOUT      	11
#define PWM_TB_SYNCIN_EPWM12_SYNCOUT      	12
#define PWM_TB_SYNCIN_EPWM13_SYNCOUT     		13
#define PWM_TB_SYNCIN_EPWM14_SYNCOUT     		14
#define PWM_TB_SYNCIN_EPWM15_SYNCOUT     		15
#define PWM_TB_SYNCIN_EPWM16_SYNCOUT     		16
#define PWM_TB_SYNCIN_ECAP1_SYNCOUT     		17
#define PWM_TB_SYNCIN_ECAP2_SYNCOUT     		18
#define PWM_TB_SYNCIN_ECAP3_SYNCOUT     		19
#define PWM_TB_SYNCIN_ECAP4_SYNCOUT     		20
#define PWM_TB_SYNCIN_ECAP5_SYNCOUT     		21
#define PWM_TB_SYNCIN_ECAP6_SYNCOUT     		22
#define PWM_TB_SYNCIN_ECAP7_SYNCOUT     		23
#define PWM_TB_SYNCIN_XBAR_INPUT5		     		24
#define PWM_TB_SYNCIN_XBAR_INPUT6		     		25
#define PWM_TB_SYNCIN_ETHERCAT_SNYC0    		26
#define PWM_TB_SYNCIN_ETHERCAT_SNYC1    		27
#define PWM_TB_SYNCIN_FSI_RX_RIG1    				31
// Zählrichtung
#define PWM_TB_COUNT_UP      								0
#define PWM_TB_COUNT_DOWN   								1
#define PWM_TB_COUNT_UPDOWN  								2
#define PWM_TB_FREEZE        								3
// Registerlademodus Period
#define PWM_TB_SHADOW     									0
#define PWM_TB_IMMEDIATE  									1
#define PWM_TB_SHDW_CTR_ZERO								0
#define PWM_TB_SHDW_CTR_ZERO_SYNC						1
#define PWM_TB_SHDW_CTR_SYNC								2
// Registerlademodus Compare
#define	PWM_CC_SHADOW												0
#define PWM_CC_IMMEDIATE  									1
#define PWM_CC_SHDW_CTR_ZERO     						0
#define PWM_CC_SHDW_CTR_PRD       					1
#define PWM_CC_SHDW_CTR_ZERO_PRD  					2
#define PWM_CC_SHDW_LOAD_DISABLE    				3
// Ausgangspinverhalten
#define PWM_AQ_NO_ACTION  									0
#define PWM_AQ_CLEAR      									1
#define PWM_AQ_SET        									2
#define PWM_AQ_TOGGLE     									3
// Totzeit-Modul
#define PWM_DB_FULL_CYCLE										0
#define PWM_DB_HALF_CYCLE										1
// Eingangssignal
#define PWM_DB_IN_A_ALL											0
#define PWM_DB_IN_A_FAEDGE_B_RIEDGE					1
#define PWM_DB_IN_A_RIEDGE_B_FAEDGE					2
#define PWM_DB_IN_B_ALL											3
// Invertierung
#define PWM_DB_POL_NONE_INV									0
#define PWM_DB_POL_A_INV										1
#define PWM_DB_POL_B_INV										2
#define PWM_DB_POL_BOTH_INV									3
// Bypass
#define PWM_DB_BOTH_BYPASSED								0
#define PWM_DB_A_BYPASSED										1
#define PWM_DB_B_BYPASSED										2
#define PWM_DB_NONE_BYPASSED								3
// A und B tauschen
#define PWM_DB_SWAP_NONE										0
#define PWM_DB_SWAP_AOUT_B_BOUT_B						1
#define PWM_DB_SWAP_AOUT_A_BOUT_A						2
#define PWM_DB_SWAP_AOUT_B_BOUT_A						3
// DC-Event Quelle Eingang
#define PWM_DC_TRIP_TRIPIN1 								0
#define PWM_DC_TRIP_TRIPIN2 								1
#define PWM_DC_TRIP_TRIPIN3 								2
#define PWM_DC_TRIP_TRIPIN4 								3
#define PWM_DC_TRIP_TRIPIN5 								4
#define PWM_DC_TRIP_TRIPIN6 								5
#define PWM_DC_TRIP_TRIPIN7 								6
#define PWM_DC_TRIP_TRIPIN8  								7
#define PWM_DC_TRIP_TRIPIN9 								8
#define PWM_DC_TRIP_TRIPIN10 								9
#define PWM_DC_TRIP_TRIPIN11 								10
#define PWM_DC_TRIP_TRIPIN12 								11
#define PWM_DC_TRIP_TRIPIN14 								13
#define PWM_DC_TRIP_TRIPIN15 								14
#define PWM_DC_TRIP_COMBINATION 						15
// DC-Event Bedingung
#define PWM_DC_EVENT_DISABLED								0
#define PWM_DC_DCXH_LOW											1
#define PWM_DC_DCXH_HIGH										2
#define PWM_DC_DCXL_LOW											3
#define PWM_DC_DCXL_HIGH										4
#define PWM_DC_DCXL_HIGH_DCXH_LOW						5
// DC-Event Filterung
#define PWM_DC_RAW_EVENT										0
#define PWM_DC_FILTERED_EVENT								1
// DC-Interrupt global Ein-/Ausschalten
#define PWM_DC_INT_DISABLE									0
#define PWM_DC_INT_ENABLE										1
// DC-OST-Interrupt Ein-/Ausschalten
#define PWM_DC_OST_INT_DISABLE							0
#define PWM_DC_OST_INT_ENABLE								1
// DC-Event Verarbeitung synchron/asynchron
#define PWM_DC_EVENT_SYNC										0
#define PWM_DC_EVENT_ASYNC									1
// DC-Event Verarbeitung unlatched/latched
#define PWM_DC_EVENT_UNLATCHED							0
#define PWM_DC_EVENT_LATCHED								1
// Tripzone-Verhalten Einstellungen
#define PWM_TZ_CONFIG_BY_TZCTL							0
#define PWM_TZ_CONFIG_BY_TZCTL2_TZCTLDCX		1
// Tripzone Ein-/Ausschalten
#define PWM_TZ_DISABLE  										0
#define PWM_TZ_ENABLE   										1
// Pin-Status bei Tripzone-Event
#define PWM_TZ_HIGH_Z       								0
#define PWM_TZ_FORCE_HI  										1
#define PWM_TZ_FORCE_LO  										2
#define PWM_TZ_NO_ACTION 										3
// Interrupt Trigger-Quelle
#define PWM_ET_DCAEVT1SOC   								0
#define PWM_ET_CTR_ZERO     								1
#define PWM_ET_CTR_PRD      								2
#define PWM_ET_CTR_PRDZERO  								3
#define PWM_ET_CTRU_CMPA    								4
#define PWM_ET_CTRD_CMPA    								5
#define PWM_ET_CTRU_CMPB    								6
#define PWM_ET_CTRD_CMPB    								7
// Anzahl an Events, bei dem ein Interrupt ausgelöst wird
#define PWM_ET_DISABLE  										0
#define PWM_ET_1ST      										1
#define PWM_ET_2ND      										2
#define PWM_ET_3RD      										3
#define PWM_ET_4TH      										4
#define PWM_ET_5TH      										5
#define PWM_ET_6TH      										6
#define PWM_ET_7TH      										7
#define PWM_ET_8TH      										8
#define PWM_ET_9TH      										9
#define PWM_ET_10TH      									  10
// SOC-Eventtrigger ein-/ausschalten
#define PWM_ET_SOC_DISABLE									0
#define PWM_ET_SOC_ENABLE										1
// Quelle SOC-Eventtrigger
#define PWM_ET_DCAEVT1   										0
#define PWM_ET_CTR_ZERO     								1
#define PWM_ET_CTR_PRD      								2
#define PWM_ET_CTR_PRDZERO  								3
#define PWM_ET_CTRU_CMPA    								4
#define PWM_ET_CTRD_CMPA    								5
#define PWM_ET_CTRU_CMPB   					 				6
#define PWM_ET_CTRD_CMPB    								7

// Wechselsperrzeit zwischen den beiden Ausgängen eines PWM-Moduls.
// Der Zahlenwert entspricht der Anzahl von Takten (SYSCLK)
// 100 Takte = 1 µs Wechselsperrzeit
// 50 Takte  = 500 ns Wechselsperrzeit
// 33 Takte  = 330 ns Wechselsperrzeit
// 30 Takte  = 300 ns Wechselsperrzeit
// 25 Takte  = 250 ns Wechselsperrzeit
// 20 Takte  = 200 ns Wechselsperrzeit
#define PWM_DEAD_BAND												25
// Periodendauer für 16 kHz Schaltfrequenz
// f_sw = (SYSCLK / EPWMCLK) * 1 / (CLKDIV * HSPCLKDIV) * 1 / (2 * PWM_PERIOD)
// Faktor 2 weil Auf-/Abzählmodus gesetzt ist
#define PWM_PERIOD													3125
// Kompensation der Synchronisationsverzögerung von ePWM2 und ePWM3
// (siehe hirzu Initialisierungsfunktion PwmInitPwm123())
// 1: CLKDIV > 1 oder HSPCLKDIV > 1
// 2: CLKDIV = 1 und  HSPCLKDIV = 1
#define PWM_SYNCHRONIZAION_DELAY						2


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion initialisiert das ePWM1, ePWM2 und ePWM3-Modul um drei Halbbrücken
// eines 3-phasigen Wechselrichter anzusteuern. Das ePWM1-Modul ist dabei der
// Master und synchronisiert die anderen zwei Halbbrücken mit sich
extern void PwmInitPwm123(void);
// Interrupt-Service-Routine des ePWM1-Moduls
__interrupt void Pwm1ISR(void);
// Funktion initialisiert das ePWM8-Modul um alle 100 ms einen Interrupt auszulösen
extern void PwmInitPwm8(void);
// Interrupt-Service-Routine des ePWM8-Moduls
__interrupt void Pwm8ISR(void);


#endif


