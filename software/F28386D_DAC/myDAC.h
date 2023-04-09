//=================================================================================================
/// @file       myDAC.h
///
/// @brief      Datei enthält Variablen und Funktionen um den internen Digital-Analog-Wandler
///							des TMS320F2838x zu nutzen. Der Code spricht das DAC-A Modul an. Die Nutzung
///							der Module B und C ist analog zu der des Moduls A. Die Referenzspannung ist
///							VREFHI und die DAC-Spannung wird an Pin ADCINA0/DACOUTA ausgegeben.
///
/// @version    V1.0
///
/// @date       08.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
#ifndef MYDAC_H_
#define MYDAC_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Referenzspannung
#define	DAC_REF_VDAC							0
#define	DAC_REF_VREFHI						1
// Laden des DAC-Werts in das aktive Register
#define DAC_SYNC_SYSCLK						0
#define DAC_SYNC_EPWM							1
// PWM-Synchronisierungsquelle
#define DAC_EPWM1SYNCPER					0
#define DAC_EPWM2SYNCPER					1
#define DAC_EPWM3SYNCPER					2
#define DAC_EPWM4SYNCPER					3
#define DAC_EPWM5SYNCPER					4
#define DAC_EPWM6SYNCPER					5
#define DAC_EPWM7SYNCPER					6
#define DAC_EPWM8SYNCPER					7
#define DAC_EPWM9SYNCPER					8
#define DAC_EPWM10SYNCPER					9
#define DAC_EPWM11SYNCPER					10
#define DAC_EPWM12SYNCPER					11
#define DAC_EPWM13SYNCPER					12
#define DAC_EPWM14SYNCPER					13
#define DAC_EPWM15SYNCPER					14
#define DAC_EPWM16SYNCPER					15
// Ausgang
#define DAC_DISABLE_OUTPUT				0
#define DAC_ENABLE_OUTPUT					1


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// Funktion initialisiert den DAC-A Modul
extern void DacAInit(void);


#endif

