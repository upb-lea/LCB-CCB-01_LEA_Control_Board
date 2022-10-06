# LEA_control_board

![](documentation/lcb_all_pcbs.png)

## Set environment variable for 3D models:

In the main menu, first select "Preferences" and then "Configure Path".
Replace the environment variable `MODEL_3D` with the current location of the 3D-models, e.g. `/path/LEA_control_board/hardware/KiCad_Library/Footprint_Library/3D_Model`. 

Note: if you are using KiCAD6, and there is a variable `KICAD6_3DMODEL_DIR`, ignore this variable and add `MODEL_3D` as mentioned above.
![](documentation/00_KiCAD_settings/3d_model_path_preferences.png)


## Flash XDS100 Firmware to FTDI Chip:

 * Download FT Prog [here](https://ftdichip.com/utilities/)
 * Load the configuration file for XDS100v2 programmer: `File` -> `Open template` -> [Modified template](/software/01_Control_boards/XDS100v2_UART.xml)
 * Flash file

Note: 
The modified file has the `Virtual COM Port` enabled to provide UART communication. This is not implemented into as into the [tutorial](https://www.youtube.com/watch?v=vZaF5ckf3OQ). To enable this, see the figure. 
![](/software/01_Control_boards/virtual_com_port.png)
