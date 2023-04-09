# Manufacturing and soldering the LCB
## Manufacturing the LCB
 * Minimum PCB thickness is 2.3 mm due to the position-pin overlapping of J501/402 and J502/J403.
 * For automated placement, the position pin holes of the connectors J501/402/J502/J403 should be milled with a depth of minimum 1.3 mm. 
 
![](/documentation/01_Control_boards/LCB-CCB-01/PCB_Manufacturing.png)

## MCU BGA soldering process
 * Remove all solder from MCU pad (smooth and flat surface).
 * Clean with PCB cleaner and remove all dust residues.
 * Apply a very thin layer of Flux on MCU pad.
 * Use the Rework station for the soldering the MCU chip.
 
 
# Initial debugger programming 
## Flash XDS100 Firmware to FTDI Chip:
 * Watch this [video](https://www.youtube.com/watch?v=vZaF5ckf3OQ) first
 * Download FT Prog [here](https://ftdichip.com/utilities/)
 * Load the configuration file for XDS100v2 programmer: `File` -> `Open template` -> [Modified template](/software/01_Control_boards/XDS100v2_UART.xml)
 * Right click on `Device: 0` -> right click -> `Apply Template` -> `Template: XDS100v2_UART.xml`
 * Flash file: `Programm Devices` -> `Program`
 
Note: 
It is very important to click on "Apply Template" as shown in the video. Otherwise the controller will not be programmed correctly!

Note: 
The modified file has the `Virtual COM Port` enabled to provide UART communication. This is not implemented as in the video. To enable this, see the figure. 
![](/software/01_Control_boards/virtual_com_port.png)
