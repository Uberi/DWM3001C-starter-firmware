/*! ----------------------------------------------------------------------------
 *  @file    ex_04a_main.c
 *  @brief   Continuous wave mode example code
 *
 *           This example code activates continuous wave mode on channel 5 for 2 minutes before stopping operation.
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) Decawave Ltd, Dublin, Ireland.
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

#if defined(TEST_CONTINUOUS_WAVE)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME "CONT WAVE v1.0"

/* Continuous wave duration, in milliseconds. */
#define CONT_WAVE_DURATION_MS 120000

/* Default communication configuration. Use channel 5 in this example as it is the recommended channel for crystal trimming operation. */
static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_1024,    /* Preamble length. Used in TX only. */
    DWT_PAC32,        /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_850K,      /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (1025 + 8 - 32),  /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

/* Recommended TX power and Pulse Generator delay values for the mode defined above. */
/* Power configuration has been specifically set for DW3000 B0 rev devices. */
extern dwt_txconfig_t txconfig_options;

/**
 * Application entry point.
 */
int continuous_wave_example(void)
{
    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* Configure SPI rate, DW3000 supports up to 36 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    while (!dwt_checkidlerc()) /* Need to make sure DW IC is in IDLE_RC before proceeding */ { };

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED     ");
        while (1) { };
    }

    /* Configure DW IC. */
    /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
    if (dwt_configure(&config))
    {
        test_run_info((unsigned char *)"CONFIG FAILED     ");
        while (1) { };
    }
    dwt_configuretxrf(&txconfig_options);

    /* Activate continuous wave mode. */
    dwt_configcwmode();

    /* Wait for the wanted duration of the continuous wave transmission. */
    Sleep(CONT_WAVE_DURATION_MS);

    /* Software reset of the DW IC to deactivate continuous wave mode and go back to default state. Initialisation and configuration should be run
     * again if one wants to get the DW IC back to normal operation. */
    dwt_softreset(1);

    /* End here. */
    while (1) { };
}
#endif
/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *    DW IC API Guide for more details on the DW IC driver functions.
 * 2. In this example, the DW IC is left in INIT state after calling dwt_initialise(), because only the slow SPI speed is used, i.e. <= 6 MHz
 ****************************************************************************************************************************************************/
