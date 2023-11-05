DWM3001C Starter Firmware
=========================

A firmware for the [Qorvo DWM3001C](https://www.qorvo.com/products/p/DWM3001C) with comprehensive examples for all of the module's UWB and ranging functionality, and developer tooling that makes working with the firmware much easier than the official tooling. Features:

* **Reproducible**: built-in Docker development environment automates away most of the fragile and finicky parts of setting up the Qorvo SDK, SEGGER Embedded Studio, SEGGER J-Link, nRF52 SDK, and nRF command line tools.
* **Minimal**: based on a heavily-modified version of the [official Qorvo DWM3001C API software](https://www.qorvo.com/products/p/DWM3001C#documents) (2022-08 version), but significantly simpler in terms of code size and organization. Many source files and folders have been consolidated to make the project much easier to navigate.
* **Complete**: includes build system, flashing tools, and logging/debugging tools. This is the only repository you need to work with the firmware.
* **Portable**: run `make save-development-environment` to generate a 5GB tar file containing the entire development environment. In 10 years from now, when half of these dependencies disappear off the internet, run `make load-development-environment` and you'll still be able to compile this project.
* Runs directly on the DWM3001C's onboard nRF52833: the official examples target an external microcontroller that then interacts with the DWM3001C, but this firmware is designed to run directly on the DWM3001C.

The firmware and instructions in this repository somewhat assume that you're using the DWM3001CDK (the official DWM3001C devkit), but you can also run it on a standalone DWM3001C simply by not using the definitions for the devkit's onboard LEDs, button, and SEGGER J-Link (see `Src/custom_board.h`).

Quickstart
----------

```sh
# MANUAL ACTION (OPTIONAL): run this command to use my prebuilt development environment, otherwise it'll be automatically built from scratch: docker pull uberi/qorvo-nrf52833-board

make build

# MANUAL ACTION: connect the lower USB port of the DWM3001CDK (labelled J9) to this computer using a USB cable (this is the J-Link's USB port)

make flash

make stream-debug-logs

# MANUAL ACTION: run the following command in another terminal to see the debug logs: tail -f Output/debug-log.txt
```

You should now see RTT output from the DWM3001CDK, try pressing the reset buttom (labelled SW1) to re-run the example program and output more logs over RTT.

Developing
----------

You'll need Docker, and many of the hardware-facing commands in the Makefile assume you're using Linux. The `make serial-terminal` command assumes you have `minicom`, `grep`, and `udevadm` installed.

You can develop your custom applications by modifying `Src/main.c` and other files within `Src/`.

Note that you'll have to manually edit `dw3000_api.emProject` with any file additions/removals/renames. It sounds annoying, and it is, but I still consider it an improvement over directly interacting with the proprietary SEGGER Embedded Studio.

License
-------

Most of the code in this repository comes from the official Qorvo SDKs and examples published on their website. Here's the copyright notice that comes with the SDKs:

> Read the header of each file to know more about the license concerning this file.
> Following licenses are used in the SDK:
> 
> * Apache-2.0 license
> * Qorvo license
> * FreeRTOS license
> * Nordic Semiconductor ASA license
> * Garmin Canada license
> * ARM Limited license
> * Third-party licenses: All third-party code contained in SDK_BSP/external (respective licenses included in each of the imported projects)
> 
> The provided HEX files were compiled using the projects located in the folders. For license and copyright information,
> see the individual .c and .h files that are included in the projects.

Therefore, you should carefully read the copyright headers of the individual source files and follow their licenses if you decide to use them. As for the parts I've built, such as the build environment, I release those under the [Creative Commons CC0 license](https://creativecommons.org/public-domain/cc0/) ("No Rights Reserved").
