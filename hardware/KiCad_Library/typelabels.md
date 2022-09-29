# Type Labels

## SMD Resistors
Differences:
 * Type: `SMD` / `THT`
 * Values: `_100R` / `__4R7` / `0R001`
 * Size: `0603` / `0704` / `0403`
 * Technology: Thinfilm `_THIN` / Thickfilm `THICK`
 * `HINT`: e.g. `SHUNT`
 * Derivation in percent: `_1P` / `_5P` / `10P` / `20P`

Examples:
 * `R_SMD_0603__THIN_100R__1P_HINT`
 * `R_SMD_0403_THICK__4R7_10P_HINT`


## THT Resistors
 Differences:
 * Values: `_100R` / `__4R7` / `0R001`
 * Size: `0603` / `0704` / `0403`
 * Derivation: `_1P` / `_5P` / `10P` / `20P`
 * RM: `_RM5` / `RM10` / `RM12`
 * `HINT`: e.g. `SHUNT`

 Examples:
 * `R_THT_0704__RM5_10P____1k`
 * `R_THT_0704_RM10__1P_0R001_SHUNT`

## Capacitors
* Type: `SMD` / `THT`
* Values: `100u` / `4u7` / `10n`
* Size: `0603` / `0704` / `0403`

Examples:
 * `C_SMD_0603__1u0`
 * `C_SMD_0805__10u`
 * `C_SMD_1206__4u7`

## Diodes
* Type: `SMD` / `THT`
* Size: `0603` / `0704` / `0403`
* Type technology: Schottky, PN, SiC, ...

## Drivers
Hard to find general rules.
* Channels: `Single`/ `Dual`

Examples:
 * Driver_Dual_TI_value
 * Driver_Single_TI_value
 * Driver_Single_TI_value

## Microcontrollers
Examples:
 * `uC_FPGA_`
 
 
## Logic ICs
Needs to be defined.


