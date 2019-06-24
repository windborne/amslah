# Amslah
A bare metal layer for the SAMD21 MCU integrated with FreeRTOS shown to improve programmer sanity.

## Getting started
To "install" Amslah, it is recommended that you add this folder to your `$PATH`; e.g. add `export PATH="/home/$USER/amslah:$PATH"` to your `.bashrc` and then don't forget to `source .bashrc`. In the spirit of laziness, the shortcut `alias alm="amslah make"` is encouraged.

Setting up an Amslah project is easy; just create a folder with your code (e.g. `main.c`) and a possibly empty `user_amslah_config.h` configuration file. Then use `amslah make` or `alm` to compile it. The other options are poorly documented if you just run `amslah`; they're somewhat better documented if you just read the Makefile yourself.

## TODO
Extremely nonexhaustive list.
 - Improve documentation.
 - Library system (that essentially just adds folders to the Makefile for both code and headers).
 - Actually try using C++.
 - Automatic switch to DFLL48M for frequencies above 8 MHz.
 - Nice way to change SPI baudrate.
