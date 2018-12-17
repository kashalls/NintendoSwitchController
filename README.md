# Switch-Controller
An ATmega32U4 Leonardo Board w/ a Serial Converter to control a Nintendo Switch over USB.

## Where and what did I use?
[Shinyquasire23's Proof of Concept - SwitchFightstick](https://github.com/shinyquagsire23/Switch-Fightstick)
[Pimanrules's Youtube Plays Series](https://www.youtube.com/user/pimanrules)
[LUFA Library](http://www.fourwalledcubicle.com/LUFA.php)
[Teensy USB UART Example](https://www.pjrc.com/teensy/uart.html)

## What do you need?
I accomplished this using a few Amazon purchases minus the fact I had previously purchased other things to attempt the same thing.

1. [A UART Serial to USB Converter](https://www.amazon.com/gp/product/B01A0BOGHG/ref=oh_aui_detailpage_o03_s00?ie=UTF8&psc=1)
    1. Make sure to check the comments, I have seen some people get non-soldered pins on some.
2. [A ATMega32U4 Leonardo Micro Board](https://www.amazon.com/gp/product/B012FOV17O/ref=oh_aui_detailpage_o04_s00?ie=UTF8&psc=1)
    1. You will have to solder the pins on this board if you need them.
3. Any Micro USB Cable that has Power, Ground and Data Lines.

# Requisites
Since this project is based off of the Switch-Fightstick example, most of this process is the same all around the boards using the LUFA Library.

> **Windows Users!**
You will need the [latest AVR toolchain](http://www.atmel.com/tools/atmelavrtoolchainforwindows.aspx) from the Atmel site.


> **Linux Users! (or Windows Users using the Linux Subsystem)**
You will need to run this command if you are running Ubuntu 8+ or your respective package manager's command!
```
sudo apt-get install gcc-avr binutils-avr avr-libc
```


> **Mac Users!**
You will need to find the AVR CrossPack or try to install avr-gcc with brew or try adding the [osx-cross/avr](https://github.com/shinyquagsire23/Switch-Fightstick/blob/master/osx-cross/avr) repository.

Just type:
```
brew tap osx-cross/avr
brew install avr-gcc
```

# How 2 Build 4 The Arduino Micro.

If you are using a Teensy 2.0 ++ board, you will need the [Teensy Loader Application](https://www.pjrc.com/teensy/loader.html).

If you are using a Arduino UNO R3 Board, you will need to set your Arduino in [DFU mode](https://www.arduino.cc/en/Hacking/DFUProgramming8U2) and flash its USB Controller with a dfu-programmer. See [the switch-fightstick information](https://github.com/shinyquagsire23/Switch-Fightstick#compiling-and-flashing-onto-the-arduino-uno-r3).

If you are using a Arduino Leonardo Board with a ATMega32U4 or related chip, follow along.

* Go ahead and clone this git library.

* Grab the UART Example Library (Version 1.1) from [here](https://www.pjrc.com/teensy/uart.html). Place the two files uart.h and uart.c in the main directory of this project.

* Grab the LUFA Library at the bottom of [this page](http://www.fourwalledcubicle.com/LUFA.php)
![LUFA Library Download Location](https://awau.moe/3be913.png)

We need the subfolder LUFA inside of this ZIP Package. Place this in the root directory of this project as our makefile requires it.

* Depending on what board you are compiling this project for, you may need to change the `makefile` setting for `MCU`. See the below table for help.

Board | Setting
------------ | -------------
Arduino UNO R3 | MCU = atmega16u2
Teensy 2.0 ++ | MCU = at90usb1286
Arduino Micro | MCU = atmega32u4
Arduino Leonardo | MCU = atmega32u4

* Compile this using your terminal by issuing `make` in the same directory of your `makefile`.
    - When this finishes without any errors, you may continue.

* Get the latest Arduino IDE from [here](https://www.arduino.cc/en/Main/Software). Try not to use the Windows App Store version as it has caused issues in the past.
    - Open the IDE and navigate to `File -> Preferences` and check `Show verbose output during: upload` and slap the `OK` button.
    - Proceed to upload any sketch (the default one works) and watch the verbose output.
    - Find the line with a `avrdude` command and copy the entire line with the options into a terminal.
        - It may look something like this: `C:\"Program Files (x86)"\Arduino\hardware\tools\avr/bin/avrdude -CC:\"Program Files (x86)"\Arduino\hardware\tools\avr/etc/avrdude.conf -v -patmega32u4 -cavr109 -PCOM4 -b57600 -D -Uflash:w:C:\User\Admin\Desktop\Switch-Controller\Joystick.hex:i`
    - Replace the everything after the `-Uflash:w:` besides the `:i` at the end with the path to the location of the Joystick.hex created when we ran `make`.
    - Make sure that `-P` is the Communication Port your Arduino Micro is currently communicating on. (The micro may show up on a different port when in programming mode!)
    - Go ahead and prepare that command and quickly press `Enter` after pulling the `RST` or `Reset Pin` to ground twice on the Arduino Micro to enter a programming mode.
    - Eventually, `avrdude` should report a success and store the command in a safe location to avoid the previous mess.

* After a successful flash, you can unplug the Micro Board and attach the UART Converter.
    - You will only need three jumpers.
        - Ground to Ground
        - TXD to RXD
        - RXD to TXD
    - Power will be handled by the Adapter and the Micro Board itself.

* Go ahead and plug your spare micro usb cable into the micro, and plug the other end into your Nintendo Switch. Your switch should recognize it and add it as a `USB` controller.

* The UART Serial Converter can now be plugged into any IoT Device with a USB Port. Use one of the examples in the examples folder to test your recreation!




