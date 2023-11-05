/*! ----------------------------------------------------------------------------
 *  @file    bandwidth_calibration.c
 *  @brief   Bandwidth calibration test code that adjusts the bandwidth according to a reference PG_COUNT value.
 *
 * @attention
 *
 * Copyright 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */

#include "deca_probe_interface.h"
#include <deca_device_api.h>
#include <deca_spi.h>
#include <example_selection.h>
#include <port.h>
#include <shared_defines.h>
#include <shared_functions.h>
#include <stdio.h>

#if defined(TEST_BW_CAL)

extern void test_run_info(unsigned char *data);

/* Example application name */
#define APP_NAME "BW CAL v1.0"

/* Start-to-start delay between frames, expressed in halves of the 499.2 MHz fundamental frequency (around 4 ns). */
/* See NOTE 6 below. */
#define CONT_FRAME_PERIOD 249600

/* Continuous frame duration, in milliseconds. */
/* See NOTE 6 below. */
#define CONT_FRAME_DURATION_MS 10000

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* No STS mode enabled (STS Mode 0). */
    DWT_STS_LEN_64,   /* STS length, see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 1 below. */
extern dwt_txconfig_t txconfig_options;

/* The frame sent in this example is an 802.15.4e standard blink. It is a 12-byte frame composed of the following fields:
 *     - byte 0: frame type (0xC5 for a blink).
 *     - byte 1: sequence number, put to 0.
 *     - byte 2 -> 9: device ID, hard coded constant in this example for simplicity.
 *     - byte 10/11: frame check-sum, automatically set by DW IC in a normal transmission and set to 0 here for simplicity.
 * See NOTE 6 below. */
static uint8_t tx_msg[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0, 0 };

/**
 * Application entry point.
 */
int bw_cal(void)
{
    /* Variable to store raw temp value */
    uint8_t raw_temp = 0;
    /* Variable to store real temp value */
    float real_temp = 0;
    /* String to contain temp value for debug purposes */
    char str_temp[32] = { 0 };

    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* Configure SPI rate, DW3000 supports up to 36 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    while (!dwt_checkidlerc()) /* Need to make sure DW IC is in IDLE_RC before proceeding */ { };

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED     ");
        while (1) { };
    }

    /* Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards. */
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

    /* Configure DW IC. See NOTE 3 below. */
    /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
    if (dwt_configure(&config))
    {
        test_run_info((unsigned char *)"CONFIG FAILED     ");
        while (1) { };
    }

    /* At this point, we are emulating a process that needs to occur at room temperature when devices are being configured in factory. */
    /* See NOTE 3 for more information */
    txconfig_options.PGcount = dwt_calcpgcount(txconfig_options.PGdly);

    /* Write the TX message into the TX buffer */
    /* This only needs to be done once if we are sending the same frame over and over again */
    dwt_writetxdata(sizeof(tx_msg), tx_msg, 0); /* Zero offset in TX buffer. */
    dwt_writetxfctrl(sizeof(tx_msg), 0, 0);     /* Zero offset in TX buffer, no ranging. */

    /* Loop forever, adjusting bandwidth periodically. */
    while (1)
    {
        /* Configure the TX spectrum parameters (power PG delay and PG Count) */
        /* See NOTE 4 below. */
        dwt_configuretxrf(&txconfig_options);

        /* START TEMPERATRE READ BLOCK */
        /* Read the raw temperature value. See  NOTE 5. */
        raw_temp = (uint8_t)(dwt_readtempvbat() >> 8);

        /* Convert raw temp value to real temp */
        real_temp = dwt_convertrawtemperature(raw_temp);

        /* Write real temp value (and current PG delay) to string */
        sprintf((char *)&str_temp, "Temp = %0.2f C, PG Delay = %d", real_temp, dwt_calcpgcount(txconfig_options.PGdly));

        /* Display Temperature value */
        test_run_info((unsigned char *)&str_temp);
        /* END TEMPERATRE READ BLOCK */

        /* START CONTINUOUS FRAME BLOCK */
        /* See NOTE 6 below. */
        /* Activate continuous frame mode. */
        dwt_configcontinuousframemode(CONT_FRAME_PERIOD);

        /* Once configured, continuous frame must be started like a normal transmission. */
        dwt_starttx(DWT_START_TX_IMMEDIATE);

        /* Wait for the required period of repeated transmission. */
        Sleep(CONT_FRAME_DURATION_MS);

        /* Disable continuous frame mode. */
        dwt_disablecontinuousframemode();
        /* END CONTINUOUS FRAME BLOCK */

        /* An additional wait is added here for debug purposes. See NOTE 7. */
        Sleep(CONT_FRAME_DURATION_MS);
    }
}

#endif
/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. Upon startup of this example, we create a structure that contains the elements necessary to configure the TX spectrum parameters. Specifically,
 *    we use the parameters necessary for configuring the device for Channel 5. The elements included in this structure are:
 *    -> PG delay of 0x34.
 *    -> TX power of 0xfdfdfdfd
 *    -> PG Count of 0x0
 *    It is important to note that the PG count is set to zero as this value must be acquired at room temperature when configuring their devices
 *    in the factory (e.g. upon first boot in factory).
 * 2. Desired configuration by user may be different to the current programmed configuration. dwt_configure is called to set desired
 *    configuration.
 * 3. It is presumed that the user (in a real device/application) will have run the dwt_calcpgcount() function at room temperature in the factory
 *    upon initialisation of their device. At that point, they will have a valid PG count value that can be used to adjust the bandwidth at other
 *    temperatures successfully. It is also presumed that this PG count value will be stored in some non-volatile memory that the device/application
 *    has access to at run-time. Using this PG count value, adjustments to bandwidth can be done at different temperatures.
 *    For the purposes of this example, the code will read the PG count value from the device (presuming that the device is at room temperature
 *    and it is fresh off the factory line) and overwrite the default value (0x0) in the txconfig_options structure. This PG count value will then
 *    be used to recalibrate the bandwidth.
 * 4. At the beginning of each loop, the bandwidth will be re-calibrated with the PG count value that was stored from the earlier dwt_calcpgcount()
 *    function call. This is done by calling the dwt_configuretxrf() function (which in turn calls the dwt_calcbandwidthadj() function). Note that
 *    this will only work if the PG count value does *NOT* equal 0. The idea is that this code will automatically calibrate the bandwidth based on
 *    the current temperature using the PG count that was obtained earlier (preferably at room temperature in factory) as a reference point.
 * 5. For the purposes of this example, the code will read the raw temperature value, convert it to a real Celsius value and display it (using
 *    test_run_info which can be 'piped' to console/LCD/etc.). These function calls (regarding temperature) are not required when re-calibrating
 *    bandwidth over temperature. They are only used here for illustration purposes.
 * 6. This example will enable continuous frame mode for the delay period. This is to output a frame that can be measured by a spectrum analyser.
 *    The details of the continuous frame mode can be found in the "ex_04b_cont_frame" example. The continuous frame mode will then be disabled
 *    after the delay period. This is done so that the bandwidth calibration can occur before starting up continuous frame mode again.
 * 7. An additional delay is added at the end of the loop for debug purposes. If a user has attached a device to a spectrum analyser, they should
 *    see a frame displayed on screen for the delay period, nothing for the same delay period, and then the re-calibrated frame should be displayed
 *    on screen. This is done so that the bandwidth of the frame can be observed over temperature.
 ****************************************************************************************************************************************************/
