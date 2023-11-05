###############################################################################
#       Nordic nRF52840-DK Build for DW3000 API + Simple Examples             #
###############################################################################
This README file will describe the following:
-> Setup
-> Compiling the code.
-> Running a Simple Example Executable.
-> Notes on the Hardware Used.

###############################################################################
#                              Setup                                          #
###############################################################################
This section will describe how a user should setup their machine in order to
utilise the code contained in this software release.

Firstly, the user should use a Windows machine in order to compile the Nordic 
nRF52840-DK port of the DW3000 API and simple examples. Decawave/Qorvo are only
supporting Windows platforms for this port at this time.

If the user does not have it installed already, they should download and
install the Segger Embedded Studio for ARM. The version used at time of writing
for this document was V5.10a, 64-bit running on Windows 10. You can find the
application at:
https://www.segger.com/products/development-tools/embedded-studio/

In order to compile the code correctly, the code will need to be compiled
against the Nordic SDK. The user will have to download this from:
https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK/Download

The version of the SDK used for the nRF52840-DK port of this software release
was the 17.0.2 nRF5 SDK. Newer versions of the SDK may work with this code, but
Decawave/Qorvo are only supporting this version of the SDK for now. Future
releases of this software package may update the Nordic nRF5 SDK version.

Please download this SDK from Nordic and unzip it to the following location:
"<DW3000 API Root Directory>/API/Build_Platforms/nRF52840-DK"

The "<DW3000 API Root Directory>" is the root directory of this code base.

Next, the user will need to do make some minor changes to the SEGGER IDE
project file in order to make sure that SEGGER will point to the right files on
their machine. The reason for this is because most of the file paths that are
set in the SEGGER IDE are relative paths.

Using a text editor application such as Notepad, Notepad++, etc., the user
should open the following file for editing:
"<DW3000 API Root Directory>/API/Build_Platforms/nRF52840-DK/
dw3000_api.emProject"
Once open, search the file for the line that contains "macros=". This line is
used to define the macros used in this project. It defines multiple project
macros all within the same line. Each macro you will need to change exists on
the same line. The user will need to change the following macros in that line:
NordicSDKDir=<DW3000 API Root Directory>/API/Build_Platforms/nRF52840-DK/SDK
DW3000APIDir=<DW3000 API Root Directory>/API

Again, "<DW3000 API Root Directory>" is the full path to the root directory of
this software package.

Once that has been completed, please save the file that is open for editing and
close it. Next the user should use Windows Explorer to navigate to the
following directory:
"<DW3000 API Root Directory>/API/Build_Platforms/nRF52840-DK"

If the user has correctly installed the SEGGER IDE, all the user will need to
do in order to load up the project is to double-click on the same
"dw3000_api.emProject" file that has just been edited.

If all has gone successfully, the SEGGER IDE should start with the "dw3000_api"
project loaded into the application.

###############################################################################
#                        Compiling the code                                   #
###############################################################################

If the SEGGER IDE has started successfully and the "dw3000_api" project has
been loaded, the next step is to compile the code using the IDE.

The code is setup in such a way that it will always compile the same "main.c"
file and generate the ELF file for flashing to the Nordic devices. There are no
separate "main.c" files for each of the simple examples. Similar to the STM
build of this code, function pointers are used to "point" the "main.c" file to
the selected simple example. In order to select the simple example the user
wishes to compile, the user needs to edit the following file appropriately:
"<DW3000 API Root Directory>/API/Src/example_selection.h"

Within that file are numerous C constants for each of the simple examples that
are available in this software package. Such as:
#define TEST_READING_DEV_ID
#define TEST_SIMPLE_TX
#define TEST_SIMPLE_RX
#define TEST_SS_TWR_INITIATOR
#define TEST_SS_TWR_RESPONDER

All the constants should be commented out EXCEPT ONE of the above. If the user
wishes to test the example that reads the DW3000 device ID, then they would
have to make sure that only the first constant in the list above is set. All
others will need to be commented out.

If the user enables multiple examples at the same time, not only will the
resulting image fail at run-time, but it may also fail to compile. Please only
select one example at at time.

Please note that the user can open this header file in any file editor / IDE
that they choose. It is not included as a file in the "dw3000_api" project
within the SEGGER IDE. The user will not find it in the "Project Items" view
which is usually on the left hand side of the screen when using the SEGGER IDE.
However, there is nothing stopping the user from including this file within the
project if they so wish. It is found upon compilation by the compiler.

Once the user has selected the simple example that they wish to compile and
build using the method described above, the next step is to build and run the
code using the SEGGER IDE.

The user can select which build configuration to
build by selecting:
"Build -> Set Active Build Configuration -> Debug"
Or
"Build -> Set Active Build Configuration -> Release"

Once the required configuration is selected, the user can select the following
to initiate the build:
"Build -> Build and Run"

this will compile the code and flash it to any Nordic nRF52840 device that is
attached via USB to the Windows machine.

###############################################################################
#                  Running a Simple Example Executable                        #
###############################################################################
The instructions above will compile and flash a simple example application to
the nRF52840-DK device. However, if the user wishes to view some of the
"printf" debug info that the applications will output, they can do the
following:
1. Connect an nRF52840-DK (with DW3000 C0 shield attached) to the Windows
   machine via a USB cable.
2. Select the required example by editing "example_selection.h" appropriately.
3. Select "Build -> Build dw3000_api".
4. When the application has finished compiling, select:
   "Debug -> Go".
5. Add breakpoints to the example code where appropriate and view "printf"
   debug info via the debug console.

###############################################################################
#                      Notes on the Hardware Used                             #
###############################################################################
This code has been developed and tested on a particular set of hardware.
Decawave/Qorvo are only supporting the hardware combination described below.
However, this code is meant to be seen as an example of how to interface with
a DW3000 device using any embedded platform. To that end, the user should use
this code as a template of how it may work on their embedded platform of
choice.

That hardware used to develop and test this source code is as follows:
-> A Nordic nRF52840-DK
-> A DW3000 C0 Arduino Shield (Available from Decawave/Qorvo)