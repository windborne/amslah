# Amslah
A bare metal layer for the SAMD21 MCU integrated with FreeRTOS shown to improve programmer sanity.

## Getting started
To "install" Amslah, it is recommended that you alias `amslah` to the `Makefile` in this repository; e.g. add `alias amslah="make -f /home/$USER/amslah/Makefile"` to your `.bashrc` and then don't forget to `source .bashrc`. In the spirit of laziness, `al` also works and is shorter.

Setting up an Amslah project is easy; just create a folder with your code (e.g. `main.c`) and a possibly empty `user_amslah_config.h` configuration file. Then use `amslah` to build the code, `amslah u` to upload it, `amslah ocd` to open openocd, and `amslah gdb` to open gdb once you have openocd running.

## TODO
Extremely nonexhaustive list.
 - Improve documentation.
 - Library system (that essentially just adds folders to the Makefile for both code and headers).
 - Actually try using C++.
 - Automatic switch to DFLL48M for frequencies above 8 MHz.
 - Nice way to change SPI baudrate.
