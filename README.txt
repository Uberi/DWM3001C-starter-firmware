=============================================================================
README.txt - Decawave's DW3XXX Application Programming Interface (API)
=============================================================================

The DW3XXX API package is composed of the following folders:

    - API/Shared/dwt_uwb_driver:

      Driver for DW3XXX UWB transceiver ICs. Details about each function can
      be found in DW3XXX API Guide.

    - API/Src/examples:

      A set of individual (simple) examples showing how to achieve different
      functionalities, e.g. sending a frame, receiving a frame, putting the
      DW3XXX to sleep, two-way ranging.  The emphasis of theses examples is
      to be readable with explanatory notes to explain and show how the main
      features of the DW3XXX work and can be used.

    - Libraries, Linkers:

      Hardware abstraction layer (system start-up code and peripheral
      drivers) for ARM Cortex-M and ST STM32 F1 processors. Provided by ST
      Microelectronics. Similar available for Nordic nRF52840 platform.

    - API/Build_Platforms/STM_Nucleo_F429/platform:
    - API/Build_Platforms/nRF52840-DK/Source/platform

      Platform dependant implementation of low-level features (IT management,
      mutex, sleep, etc).

Please refer to DW3XXX API Guide accompanying this package for more details
about provided API and examples.

NOTE: The DW3XXX API/driver code included in this package is an unbundled
      version of the DW3XXX API/driver. This version may be different to
      (generally by being newer than) those bundled with Decawave's other
      products. This particular release covers the DW3XXX hardware.

=============================================================================
=                                                                           =
=                               Release Notes                               =
=                                                                           =
=============================================================================

=============================================================================
Package v9.3 / Driver v06.00.07  (19th Nov 2021)
=============================================================================
The following updates mentioned below are included in this release.

a) Updated the driver library to 6.0.7

=============================================================================
Package v9.2 / Driver v06.00.06  (1st Oct 2021)
=============================================================================
The following updates mentioned below are included in this release.

a) Updated the SFD holdoff configuration to be restored to default
   if preamble length <= 64, this will make sure correct configuration is applied
   when dwt_configure() is called multiple times without device reset.

=============================================================================
Package v9.1 / Driver v06.00.05  (16th Sept 2021)
=============================================================================
The following updates mentioned below are included in this release.

a) Updated PLL lock wait time to 1ms, this is the max time application/host
   should wait to allow PLL to lock. In general the lock time is < 100 us, 
   however on some slow-corner parts the time may take up to 900 us!

=============================================================================
Package v9.0 / Driver v06.00.04  (17th August 2021)
=============================================================================
The following updates mentioned below are included in this release.

a) Various API calls have been updated to be compatible with latest versions of
   the driver APIs.

b) NLOS (Non Line-Of-Sight) example has been added to project.

c) TX Power adjustment example has been added to project.

d) AES-CCM* example added to illustrate how to use AES-CCM* correctly. The
   example uses the test vectors defined in the IEEE 802.15-4-2020 standard.

e) Design of dwt_probe() API changed since last release. The Simple Examples
   have been updated accordingly to cater for this.

f) Various coding style changes made throughout to give code a singular coding
   style.

g) Fix for SP3 packets RX events handling on DW3720 devices in dwt_isr() API.

h) Refactoring done to unify the different driver versions into one.

i) dwt_ds_sema_request() function now no longer returns a value.

j) ADC threshold has been reverted back to ADC_TH_TIE default.

k) For longer Ipatov preamble of > 64, the dwt_configure() now correctly sets
   the number of symbols of accumulation to wait before checking for an SFD
   pattern.

l) The following new APIs have been added to the driver:

    i)   dwt_set_alternative_pulse_shape() - Set alternate pulse shape
         according to ARIB.
    ii)  dwt_entersleepafter() - Auto enter sleep / deep-sleep after TX.
    iii) dwt_ds_sema_status_hi() - Read semaphore status high byte.
    iv)  dwt_ds_setinterrupt_SPIxavailable() - Enable/disable events on
         SPI1MAVAIL or SPI2MAVAIL events.
    v)   dwt_update_dw() - Update device pointer used by interrupt.
    vi)  dwt_nlos_alldiag() - Determine if received signals (Ipatov/STS1/STS2)
         is LOS (Line-Of-Sight) or NLOS (Non-Line-Of-Sight).
    vii) dwt_nlos_ipdiag() - Determine if received Ipatov signal is LOS
         (Line-Of-Sight) or NLOS (Non-Line-Of-Sight).
    viii)dwt_adjust_tx_power() - calculate TX power setting by applying boost.

    NOTE: The following additional APIs are for debug use only. They are only
          used by DecaRanging.

    ix)  dwt_getrxantennadelay() - Get current 16-bit RX antenna delay value.
    x)   dwt_gettxantennadelay() - Get current 16-bit TX antenna delay value.
    xi)  dwt_readctrdbg() - Get low 32-bits of STS IV counter.
    xii) dwt_readdgcdbg() - Get 32-bit value of DGC debug register.
    xiii)dwt_readCIAversion() - Get CIA image version written to chip.
    xiv) dwt_getcirregaddress() - Get CIR base address.
    xv)  dwt_get_reg_names() - return list of regsiter name/value pairs for
         debug.

m) Handling of uncorrectable errors in PHR has been added. This stops frame
   lengths being reported as zero in some cases.

n) A compilation flag (_DGB_LOG) has been added to allow for debug strings to
   be enabled/disabled as a project needs them.

o) Fixes added for dwt_isr() when STS No Data packets are used. There was a
   potential for regular frames to be treated as STS No Data packets.

p) WIN32 compilation of driver has been cleaned up to be more streamlined.

q) dwt_setcallbacks() has an additional input argument for a pointer to a
   callback function for Dual SPI events.

r) dwt_softreset() now has an input parameter. It resets the semaphore. This
   change is only valid for DW3720 devices.

s) Fix added for dwt_readdiagnostics() API. Some values in API were not being
   read correctly from registers.

t) dwt_setgpiomode() API has been refactored to make is easier to use. Input
   parameters have changed.

u) Improvements made for large, varying range values on Channel 9.

=============================================================================
Package v8.1 / Driver v05.05.04  (02nd June 2021)
=============================================================================
In addition to the existing updated driver layout for ER2.1 release. Following
updates mentioned below are included in this release changed from ER2.1 release.

a) Added an API to set “Fixed STS” - the same STS will be sent in each packet.

b) Disable ADC count warning check in DW3xxx, flag was raised in error.

=============================================================================
Package v8.0 / Driver v02.01.00  (06th May 2021)
=============================================================================
In addition to the existing updated driver layout for ER2.0 release. Following
updates mentioned below are included in this release changed from ER1 release.

a) Enabling the second SPI support on ST Micro and the Nordic nRF52840-DK.
   Changes in main.c() for STM32 to initialise second SPI and changes in file
   “deca_spi.c” for the initialisation of second SPI in
   “Build_Platforms\nRF52840-DK\Source\platform”.

b) Disable Pre-buffer-enable for Ch9 PLL calibration for E0.

c) Bug in deca_compat.c where dwt_read_reg() was returning 0.

d) Fixing function that write the PDoA Offset in deca_compat.c under
   case: “DWT_SETPDOAOFFSET”. The value to be written is ANDed with the
   inverse of CIA_ADJUST_PDOA_ADJ_OFFSET_BIT_MASK.

e) Simple Examples updated are same as ER2.0 except:
   1) Double Buffer RX example:
      Second SPI support enabled for both ST Micro and the Nordic nRF52840-DK.
   2) Read Dev ID:
      Second SPI support enabled for both ST Micro and the Nordic nRF52840-DK.
   3) Simple TX:
      Second SPI support enabled for both ST Micro and the Nordic nRF52840-DK.
   4) Double Sided Two Way Ranging with STS example:
      Failure monitored on nRF is now fixed.


=============================================================================
Package v7.0 / Driver v02.00.00  (13th April 2021)
=============================================================================

In addition to the existing updated driver layout for CR5 release. Following
updates mentioned below are included in this release changed from ER1 release.

a) Update in the function dwt_setgpiomode():
   This function will now // clear GPIO which has been specified by the mode
   (Each mode takes up 3 bits) and set GPIO mask as specified to enable it for
   individual GPIO or multiple GPIOs.

b) Update in the dwt_isr() function:
   Clear SPI RDY events after the callback - this lets the host read the
   SYS_STATUS register inside the callback "after" calling the corresponding
   callback if present.

c) Fixes for PLL re-calibration. PLL_CAL_EN_BIT needs to be set when PLL is
   calibrated. Updated dwt_setdwstate().

d) Wakeup (from sleep) times have been shortened from 500us to 200us.

e) Updated STS_CONFIG_HI_RES – the STS threshold to >=60%, so that only if
   STS qual is < 60%, the STS_CQ_EN bit will be set in the STS_TOAST register.

f) New DGC LUTs, change in the OTP values, the dwt_initialise() has been
   updated to check the OTP contains the new LUTs, and only if it does the
   driver will load/kick them from OTP, otherwise the values in the driver
   (refer API guide for more information) be loaded/written into the device.

g) Added new dwt_adcoffsetscalibration() API which is called at the end of
   dwt_configure to optimize receiver / configure optimal ADC thresholds.

h) _dwt_adjust_delaytime() – this has been updated for the delayed TX w.r.t
   RX timestamp. As TX/RX antenna delay is automatically added to the TX/RX
   timestamp, the antenna delay is subtracted from the delayed TX time.

i) Only disable STS CMF when PDOA mode 1 is used.

j) Fix for channel power parameter in the diagnostic structure, the parameter
   has been changed to 32 bits as the corresponding bitfield has been
   increased to 20-bits.

k) Simple Examples updated are same as CR5 except:
   1) Double Buffer RX example:
      This example has a check for device ID. If the device ID is not from E0
      samples. The test will go into a forever while loop.

   2) Timer example:
      This example has a check for device ID. If the device ID is not from E0
      samples. The test will go into a forever while loop.

   3) GPIO example:
      This example has a check for device ID to select the GPIO mask for C0
      and E0. If the device ID is not from E0 samples. User will be required
      to select the single GPIO (1,2,3 ..) or multiple GPIOs (GPIO_ALL) from
      enum that needs to be enabled.


=============================================================================
Package v6.0 / Driver v05.00.00  (10th October 2019)
=============================================================================

a) New driver layout has been added as part of this release. A "compatibility
   layer" is now in use. Each example now requires a dwt_probe() API call
   before dwt_initialise() API call. In previous releases, dwt_initialise()
   was always the first API to be called. This is no longer the case. Please
   see API Guide for more information.

b) Issues with XTAL trim values have been fixed.

c) Simple Examples with STS enabled will now check STS status and STS quality.
   STS status checking will not work if the Ipatov preamble length is 64 or
   less. In this case, the user should only check for STS quality.

d) Various fixes to SS-TWR and DS-TWR examples to use a single delay value
   between RX/TX rather than different values for different supported
   platforms.

e) Simple Examples will no longer reference deca_regs.h directly. Various new
   APIs have been created to accommodate for this. Please see API Guide for
   more information.

f) Simple Examples release has removed some unnecessary files and changed the
   layout of the code. Each supported build platform (STM & Nordic) now has
   its own directory.

g) The “DWT_PGFCAL” argument has been added to all dwt_configuresleep() API
   calls in the example code. This allows for the receiver to be re-enabled
   after sleep (on-wake).

h) Wakeup (from sleep) times have been shortened from 500us to 200us.

i) Interrupt flag names have changed slightly from previous release. For
   example, “SYS_ENABLE_LO_TXFRS_ENABLE_BIT_MASK” is now
   “DWT_INT_TXFRS_BIT_MASK”.

j) New Simple Examples added:
   1) PLL calibration example:
      This example will monitor the temperature of the device. If there is a
      significant change in temperature, the PLL is re-calibrated.
   2) Bandwidth calibration example:
      This example will record the initial PG count (emulating what should be
      done in factory). The example will recalibrate the bandwidth given this
      reference PG count value in a loop over time. The example should be run
      in a temperature chamber over a range of operating temperatures. The
      device will output a continuous frame for bandwidth monitoring on a
      spectrum analyser.

k) Simple Example changes:
   1) Continuous frame example:
      The continuous frame example will now disable the transmission of the
      frame before resetting. Users can add a breakpoint in to monitor this on
      a spectrum analyser.


=============================================================================
Package v5.0 / Driver v01.03.12  (10th October 2019)
=============================================================================

a) Fix for gearing table selection and SCP coding threshold, VCM and TX power
values have been increased by 3dB to improve performance.

b) Wakeup pin functionality tested. Wake up pin is configured as FORCE_ON by
default. It needs to be reconfigured to WAKE_UP before wake-up functionality
can be tested.

c) Fixed dwt_entersleepaftertx - which was setting a wrong bit.

d) PLEN 32 was configuring the device to transmit 40 symbols. This has been
fixed.

e) Added multi-config IEEE 802.15.4 (BPRF) example that can use 32 different
configurations.

=============================================================================
Package v4.0 / Driver v01.03.0b  (31st May 2019)
=============================================================================

a) Updated CIA configurations, this improved timestamping accuracy.

b) Updated PLL cal, a manual cal is necessary to ensure proper PLL
calibration/locking before device can switch to IDLE_PLL state.

c) Added example TX_SLEEP with auto PLL cal – this can be used to show how
device may stay in IDLE_RC after wakeup and then automatically calibrate PLL
just before TX.

d) Added IEEE 802.15.4 (BPRF) examples (simple TX and RX with new cipher
configuration, 1g and 2g)
.

e) Added SS TWR with IEEE 802.15.4 (BPRF) configuration example.

f) Added RX Sniff mode example.

=============================================================================
Package v3.0 / Driver v01.03.07  (15th May 2019)
=============================================================================
This has new and updated examples, and updated APIs – API version is 1.03.07:

a) When operating in mode 2 cipher PRF was incorrectly set. It needs to match
Ipatov PRF of 64 MHz.

b) Fix LFSR advance API (dwt_cplfsradvance).

c) Updated dwt_configure() and dwt_enable_rf_tx() APIs – correct PDOA switch
configuration for AOA devices.

d) Added example 1d: TX timed SLEEP.

e) Added example 13a: GPIO example.

=============================================================================
Package v2.0 / Driver v01.03.04  (12th April 2019)
=============================================================================
This has new and updated examples, and updated APIs – API version is 1.03.04:

a) Fix for PLL calibration procedure for channel 9.

b) Updated AES APIs.

c) Updated CF and CW APIs.

d) SS-TWR examples have optimised response times.

e) Added example 5a/5b: DS-TWR.

f) Added example 2f: RX with XTAL trim.

g) Added example 7a/7b: TX and auto-ACK.

h) Added example 1e: TX with CCA example.

=============================================================================
Package v1.0 / Driver v01.03.00  (29th March 2019)
=============================================================================

a) Initial release of DW3000 Driver and API, with some simple example
projects.

=============================================================================
