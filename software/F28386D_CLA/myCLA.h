//=================================================================================================
/// @file       myCLA.c
///
/// @brief      Datei enthält Variablen und Funktionen um die Funktion des CLA-Moduls eines
///							TMS320F2838x zu demonstrieren. Es werden drei CLA-Tasks implementiert: Task 1
///							initialisiert das CLA-Modul. Task 2 liest und beschreibt Peripherie-Register
///							(ADC und ePWM). Task 3 führt eine einfache Rechenoperation aus.
///
/// @version    V1.1
///
/// @date       13.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
#ifndef MYCLA_H_
#define MYCLA_H_
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myDevice.h"


//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// Ab C2000Ware Version 3.03 gibt es das Struct "Cla1OnlyRegs" nicht mehr.
// Als Abhilfe muss ein Pointer auf das Struct "CLA_ONLY_REGS" erstellt werden
#define Cla1OnlyRegs ((volatile struct CLA_ONLY_REGS *)(uintptr_t)0x0C00U)
// Triggerquelle für CLA-Tasks
#define CLA_TASK_TRIGGER_SOFTWARE										0
#define CLA_TASK_TRIGGER_ADCA_INT1									1
#define CLA_TASK_TRIGGER_ADCA_INT2									2
#define CLA_TASK_TRIGGER_ADCA_INT3									3
#define CLA_TASK_TRIGGER_ADCA_INT4									4
#define CLA_TASK_TRIGGER_ADCA_EVT_INT								5
#define CLA_TASK_TRIGGER_ADCB_INT1									6
#define CLA_TASK_TRIGGER_ADCB_INT2									7
#define CLA_TASK_TRIGGER_ADCB_INT3									8
#define CLA_TASK_TRIGGER_ADCB_INT4									9
#define CLA_TASK_TRIGGER_ADCB_EVT_INT								10
#define CLA_TASK_TRIGGER_ADCC_INT1									11
#define CLA_TASK_TRIGGER_ADCC_INT2									12
#define CLA_TASK_TRIGGER_ADCC_INT3									13
#define CLA_TASK_TRIGGER_ADCC_INT4									14
#define CLA_TASK_TRIGGER_ADCC_EVT_INT								15
#define CLA_TASK_TRIGGER_ADCD_INT1									16
#define CLA_TASK_TRIGGER_ADCD_INT2									17
#define CLA_TASK_TRIGGER_ADCD_INT3									18
#define CLA_TASK_TRIGGER_ADCD_INT4									19
#define CLA_TASK_TRIGGER_ADCD_EVT_INT								20
#define CLA_TASK_TRIGGER_XINT1											29
#define CLA_TASK_TRIGGER_XINT2											30
#define CLA_TASK_TRIGGER_XINT3											31
#define CLA_TASK_TRIGGER_XINT4											32
#define CLA_TASK_TRIGGER_XINT5											33
#define CLA_TASK_TRIGGER_EPWM1_INT									36
#define CLA_TASK_TRIGGER_EPWM2_INT									37
#define CLA_TASK_TRIGGER_EPWM3_INT									38
#define CLA_TASK_TRIGGER_EPWM4_INT									39
#define CLA_TASK_TRIGGER_EPWM5_INT									40
#define CLA_TASK_TRIGGER_EPWM6_INT									41
#define CLA_TASK_TRIGGER_EPWM7_INT									42
#define CLA_TASK_TRIGGER_EPWM8_INT									43
#define CLA_TASK_TRIGGER_EPWM9_INT									44
#define CLA_TASK_TRIGGER_EPWM10_INT									45
#define CLA_TASK_TRIGGER_EPWM11_INT									46
#define CLA_TASK_TRIGGER_EPWM12_INT									47
#define CLA_TASK_TRIGGER_EPWM13_INT									48
#define CLA_TASK_TRIGGER_EPWM14_INT									49
#define CLA_TASK_TRIGGER_EPWM15_INT									50
#define CLA_TASK_TRIGGER_EPWM16_INT									51


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Die folgenden Variablen müssen zusätzlich noch in der main.c mit einem #pragma-Befehl
// deklariert werden, in welchem der Speicherbereich zugeordnet wird (CPU to CLA oder
// CLA to CPU)
extern unsigned int cpuToCla;
extern unsigned int claToCpu;


//-------------------------------------------------------------------------------------------------
// Prototypes of global functions
//-------------------------------------------------------------------------------------------------
// CLA-Task 1. Dient als Initialisierungs-Task für das CLA-Modul,
// da die CPU auf manche CLA-Register nicht zugreifen kann
__interrupt void ClaTask1(void);
// CLA-Task 2. Dient zur Demonstration von Peripherie-getriggerten Tasks
// und dem Zugriff von CLA auf bestimmte Peripherie (ADC, PWM, CMPSS)
__interrupt void ClaTask2(void);
// CLA-Task 3. Dient zur Demonstration von Software-getriggerten
// Tasks und dem Austausch von Daten zwischen CPU und CLA
__interrupt void ClaTask3(void);


#endif
