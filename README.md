# Amslah
A bare metal layer for the SAMD21 MCU integrated with FreeRTOS shown to improve programmer sanity.

## Getting started
To "install" Amslah, it is recommended that you alias `amslah` to the `Makefile` in this repository; e.g. add `alias amslah="make -f /home/$USER/amslah/Makefile"` to your `.bashrc` and then don't forget to `source .bashrc`. In the spirit of laziness, `al` also works and is shorter.

Setting up an Amslah project is easy; just create a folder with your code (e.g. `main.c`) and a possibly empty `user_amslah_config.h` configuration file. Then use `amslah` to build the code, `amslah u` to upload it, `amslah ocd` to open openocd, and `amslah gdb` to open gdb once you have openocd running. If you are debugging a non samd51 you need to edit the amslah makefile to use the correct config file

### Help!
Make an issue or ask Joa(h)n and we'll be happy to help. Everything in the repo has been tested but it might not be as self-documenting as it is in my head.

### Help?
Yes, please! There's always things to do in the goal of getting a friendly realtime system for the SAMD chips. If you're bored, you can participate in the upcoming rewrites in Rust, Haskell, and ATS.

### What's with the name?
Sadly that's a trade secret. Ask in person.

## TODO
Extremely nonexhaustive list.
 - SAMD51 support.
 - Improve documentation.
 - Automatic switch to DFLL48M for frequencies above 8 MHz.
