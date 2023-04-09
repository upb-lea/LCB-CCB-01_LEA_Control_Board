//=================================================================================================
/// @file       myIPC.c
///
/// @brief      Datei enthält Variablen und Funktionen um das IPC-Modul (Inter Processor
///							Communication) des Mikrocontrollers  TMS320F2838x zu konfigurieren um
///						 	Kommunikation zwischen CPU 1 und CPU 2 zu ermöglichen.
///
/// @version    V1.0
///
/// @date       19.09.2022
///
/// @author     Daniel Urbaneck
//=================================================================================================
//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "myIPC.h"


//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
// Speichert die von CPU 2 empfangenen Daten
uint32_t ipcDataFromCPU2 = 0;


//-------------------------------------------------------------------------------------------------
// Global functions
//-------------------------------------------------------------------------------------------------
//=== Function: IpcInit ===========================================================================
///
/// @brief  Funktion initialisiert das IPC-Modul
///
/// @param  void
///
/// @return void
///
//=================================================================================================
void IpcInit(void)
{
    // Register-Schreibschutz ausschalten
    EALLOW;

		// CPU-Interrupts während der Konfiguration global sperren
    DINT;
		// Interrupt-Service-Routinen für den CIPC0_INT-Interrupt an die entsprechende
    // Stelle (Interrupt) der PIE-Vector Table speichern.
    // Für die Kommunikation CPU 2 -> CPU 1 stehen 4 Interrupts zur Verfügung (CIPC0
    // ... CIPC3). Diese werden ausgelöst, wenn CPU2 das Flag IPC0 ... IPC3 setzt
    // (CPU2TOCPU1IPCSET.bit.IPCx = 1). Das Flag wird gelöscht, sobald CPU 1 das
    // Flag quittiert (CPU1TOCPU2IPCACK.bit.IPCx = 1) oder CPU2 das Flag selber
    // wieder löscht (CPU2TOCPU1IPCCLR.bit.IPCx = 1, siehe S. 1933 Reference
    // Manual TMS320F2838x). Das gesamte Verhalten ist identisch bei CPU 2. Trotz
    // der gleichen Interrupt-Namen (CIPC0_INT ... CIPC3_INT) haben die Interrupts
    // zwischen CPU 1 und CPU 2 keine Verbindung, d.h. der Eintritt in die ISR von
    // CPU1-CIPC0_INT und das löschen der entsprechenden Flags hat keine Auswirkung
    // auf den CPU2-CIPC0_INT, da beiede CPUs eigene und voneinander unabhängige
    // PIE-Module haben (siehe S. 146 Reference Manual TMS320F2838x)
    PieVectTable.CIPC0_INT = &IPC0ISR;
    // CIPC0_INT-Interrupt freischalten  (Zeile 1, Spalte 13 der Tabelle)
    // (siehe PIE-Vector Table S. 150 Reference Manual TMS320F2838x)
    PieCtrlRegs.PIEIER1.bit.INTx13 = 1;
    // CPU-Interrupt 1 einschalten (Zeile 1 der Tabelle)
    IER |= M_INT1;
    // Interrupts global einschalten
    EINT;

    // Register-Schreibschutz setzen
    EDIS;
}


//=== Function: IpcSendDataToCpu2 =================================================================
///
/// @brief  Funktion sendet Daten an CPU 2
///
/// @param  void
///
/// @return uint32_t data
///
//=================================================================================================
void IpcSendDataToCpu2(uint32_t data)
{
    // Daten an CPU 2 senden:
    // Gewünschte Daten in eines/alle der drei Register schreiben. Alle drei
    // Register sind identisch aufgebaut und können gleich genutzt werden.
    // Die unterschiedliche Bennenung ergibt sich aus der von TI vorge-
    // schlagenen Nutzung (siehe S. 1935 Reference Manual TMS320F2838x)
    Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCSENDDATA = data;
    //Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCSENDCOM  = 0xFFFF;
    //Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCSENDADDR = 0xFFFF;
    // Flag setzen um CPU 2 neue Daten zu signalisieren. Falls
    // konfiguriert, wird dadurch bei CPU 2 ein Interrupt ausgelöst
    Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCSET.bit.IPC0 = 1;
}


//=== Function: IPC0ISR ===========================================================================
///
/// @brief	ISR wird aufgerufen, wenn CPU 2 das Flag IPC0 setzt. Kann zur Signalisierung eines
///					Datenaustauschs zwischen CPU 1 und CPU 2 genutzt werden.
///
/// @param  void
///
/// @return void
///
//=================================================================================================
__interrupt void IPC0ISR(void)
{
		// Von CPU 2 empangene Daten auslesen. Die Daten in diesem Register sind sofort nach dem
		// Schreiben (von CPU 2) für CPU 1 sichtbar, es wäre kein Interrupt zum Auslesen nötig.
		// Der Interrupt dient hier nur als Vermeidung des ansonsten nowendigen Register-Pollings.
		ipcDataFromCPU2 = Cpu1toCpu2IpcRegs.CPU2TOCPU1IPCRECVDATA;
		// Flag quittieren (löscht das Flag CPU2toCPU1Flag0)
		Cpu1toCpu2IpcRegs.CPU1TOCPU2IPCACK.bit.IPC0 = 1;
		// Interrupt-Flag der Gruppe 1 löschen (da gehört der CIPC0_INT-Interrupt zu)
		PieCtrlRegs.PIEACK.bit.ACK1 = 1;
}



