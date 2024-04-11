# Writing to SSD1306 using bare metal on STM32F411E Discovery Board

This repo is for learning how to use the I2C library on the STM32F411E Discovery Board without the use of
the HAL library. The purpose of this project was to gain a better understanding of the I2C library, interrupts, and exploring using VS Code as the main IDE, instead of Keil or CubeProgrammer.

## Setup

This repo needs the following extensions in order to convert a Keil uVision project to a CMSIS solution (csolution project)[^1]. More details on setup can be found from ARM's guide listed below.

VS Code Extensions:
- Arm Keil Studio Pack
- Arm CMSIS csolution
- Arm Debugger
- Arm Device Manager
- Arm Environment Manager
- ASM Virtual Hardware

[^1]: [ARM Keil Studio Visual Studio Code Extensions User Guide](https://developer.arm.com/documentation/108029/0000/Get-started-with-an-example-project)

With these extensions and the csolution created, a Keil like project can be created in VS Code. VS Code will need to create the following files in order to debug:

```
.vscode
|- launch.json
|- tasks.json
|- settings.json
```

These files are necessary to launch the debugger, flash the device, and configure other libraries that may be needed. 

### Troubleshooting

If the target fails to flash, a configuration may need to be setup:

Manage CMSIS Solution -> Run and Debug -> Run Configuration -> Add new (ST-Link)

Under .csolution add Target type
```
  - type: Debug
    device: STM32F401VCTx
    optimize: none
```

## Building, flashing, and debugging

This project can be built, flashed, and debugged using the CMSIS extention provided by ARM. This extension references the cproject yaml file which can be configured by groups to add source or include files. The built in debugger allows for slightly similar debugging provided in the Keil IDE and is efficient enough for basic debugging through the STM32's ST-Link. 

In addition to the CMSIS debugger, a logic analyzer was also used to check the I2C packets being sent to the STM32. With the logic analyzer, the details of the packets (i.e. memory address, data being sent, ack/nack), can be verified.

## Components

This project uses the SSD1306 OLED display model and the Reference sheet can be found here[^2].

[^2]: [SSD1306 Reference Sheet](http://www.lcdwiki.com/res/MC091GX/SSD1306-Revision%201.5.pdf)

In addition to the STM32F411E_DISCO board, this project uses the following components:

  * 10k resistor 		- x2
	* 1k resistor 		- x1
	* LED							- x1
	* Push Buttons		- x2
	* Jumper Cables

### Pin mapping

The pin mapping for the project is listed below:

```
I2C Pinout
SDA - PB9
SCL - PB8

GPIO Pinout
LED - PC7
Set Button - PA8
Reset Button - PA4
```

## Additional information

I2C configuration set up referencing both the HAL library and ControllersTech's article [^3] and WeewStack's [^4] guide.

[^3]: [ControllerTech's STM32 I2C Configuration using Registers](https://controllerstech.com/stm32-i2c-configuration-using-registers/)

[^4]: [WeewStack's Guide](https://github.com/weewStack/STM32F1-Tutorial/tree/master/060-STM32F1_I2C_LIBRARY_SETUP)

The STM32-SSD1306 library [^5], that uses the HAL library, was referenced in order to set up the screen for I2C. 
[^5]: [STM32-SSD1306 library](https://github.com/afiskon/stm32-ssd1306/tree/master)

By breaking down the HAL library, this project was able to successfully communicate with the SSD1306 by only writing to resistors of the STM32.

Another aspect of the project was to upload images and animations and use external buttons to trigger interrupts to move these images left and right within the bounds of the screen.

The images from this project can be found:

```
https://64.media.tumblr.com/da5c4f63dfef5de4ef3e369b0a3cc81b/tumblr_owz82zeYIE1twukhxo1_540.pnj
https://www.reddit.com/r/PixelArt/comments/xzdhso/tiiiny_super_tiny_1bit_pixel_art_characters_design/#lightbox
```

Another useful file to reference was the `stm32f411xe.h` file as it contained the alias for the registers

### Known bugs

After setting animation, moving an animation causes the image to move left/right, but does not resume animation after interrupt occurs
