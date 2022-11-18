# Type Labels

## SMD Resistors
Differences:
 * Type: `SMD` / `THT`
 * Values: `_100R` / `__4R7` / `0R001`
 * Size: `0603` / `0704` / `0403`
 * Technology: Thinfilm `_THIN` / Thickfilm `THICK`
 * `HINT`: e.g. `SHUNT`
 * Derivation in percent: `_1P` / `_5P` / `10P` / `20P`
 * Voltag: `75V` / `100V`

Examples:
 * `R_0805_100R__THIN_20P_150V`
 * `R_0603_100R__THIN__1P__75V`
 * `R_0403__4R7_THICK_10P__50V`


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
* Voltage: `25V` / `100V` / `600V`

Examples:
 * `C_SMD_0603_100V__1u0`
 * `C_SMD_0805__25V__10u`
 * `C_SMD_1206__16V__4u7`

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


