/*! ----------------------------------------------------------------------------
 *  @file    tx_power_adjustment_example.c
 *  @brief   This example demonstrates how to the dwt_adjust_tx_power API can be used to apply
 *           a boost on top of a reference TxPower setting.
 *
 *           1. EXHAUSTIVE_API_TEST can be set to '1' to perform an exhaustive test of the API.
 *
 *           2. By default, the example will perform a simple Tx with adjusted TxPower relatively to the reference Tx power.
 *
 *
 *
 *
 *
 * @attention
 *
 * Copyright 2021 (c) Qorvo Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Qorvo
 */
#include "deca_probe_interface.h"
#include <deca_device_api.h>
#include <deca_spi.h>
#include <example_selection.h>
#include <port.h>
#include <shared_defines.h>
#include <shared_functions.h>

#if defined(TEST_TX_POWER_ADJUSTMENT)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME "TX_POWER_ADJUSTMENT        v1.0 \r\n"

#define STR_SIZE 256

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

/* The frame sent in this example is an 802.15.4e standard blink. It is a 12-byte frame composed of the following fields:
 *     - byte 0: frame type (0xC5 for a blink).
 *     - byte 1: sequence number, incremented for each new frame.
 *     - byte 2 -> 9: device ID, see NOTE 1 below.
 */
static uint8_t tx_msg[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E' };
/* Index to access to sequence number of the blink frame in the tx_msg array. */
#define BLINK_FRAME_SN_IDX 1

#define FRAME_LENGTH (sizeof(tx_msg) + FCS_LEN) // The real length that is going to be transmitted

#define FRAME_DURATION 178 // Frame duration for PLEN 128, 6M8, 12 bytes data

#define TX_DELAY_MS 500

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 2 below. */
extern dwt_txconfig_t txconfig_options;

/**
 * Application entry point.
 */
void tx_power_adjustment_example(void)
{
    int err;
    uint32_t ref_tx_power = 0x36363636; // Base TxPower setting. See NOTE 6 below.
    uint32_t adj_tx_power;
    uint16_t boost;
    uint16_t applied_boost;
    dwt_txconfig_t tx_config;

    unsigned char str[STR_SIZE];

    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* Configure SPI rate, DW3000 supports up to 36 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    /* Target specific drive of RSTn line into DW IC low for a period. */
    reset_DWIC();

    /* Time needed for DW3000 to start up
     * (transition from INIT_RC to IDLE_RC) */
    Sleep(2);

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    /* Need to make sure DW IC is in IDLE_RC before proceeding */
    while (!dwt_checkidlerc()) { };

    if (dwt_initialise(DWT_DW_IDLE) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED     ");
        while (1) { };
    }

    /* Configure DW IC. See NOTE 5 below. */
    /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
    if (dwt_configure(&config))
    {
        test_run_info((unsigned char *)"CONFIG FAILED     ");
        while (1) { };
    }

    /* Using application service to calculate duration of Tx frame duration */

    /* Using application service to calculate the boost allowed in function of Tx frame duration */
    /* This service calculates a boost relatively to a 1ms frame*/
    boost = (uint16_t)calculate_power_boost(FRAME_DURATION);

    /* Using D3XXX driver API to calculate the TxPower setting corresponding to a reference TxPower + boost*/
    err = dwt_adjust_tx_power(boost, ref_tx_power, config.chan, &adj_tx_power, &applied_boost);

    if (err == DWT_ERROR)
    {
        test_run_info((unsigned char *)"Cannot calculated adjusted TXPower for boost and ref_tx_power parameters.");
        while (1) { };
    }

    tx_config.power = adj_tx_power;
    tx_config.PGcount = txconfig_options.PGcount;
    tx_config.PGdly = txconfig_options.PGdly;

    Sleep(1000);
    memset(str, 0, STR_SIZE);
    snprintf((char *)str, STR_SIZE, "Reference_tx_power:%lx; Boost:%d; Adjusted_tx_power:%lx\r\n", ref_tx_power, boost, adj_tx_power);
    test_run_info(str);
    Sleep(1000);

    /* Configure the TX spectrum parameters (power PG delay and PG Count) */
    dwt_configuretxrf(&tx_config);

    /* Loop forever sending frames periodically. */
    while (1)
    {
        /* Write frame data to DW IC and prepare transmission. See NOTE 3 below.*/
        dwt_writetxdata(FRAME_LENGTH - FCS_LEN, tx_msg, 0); /* Zero offset in TX buffer. */

        /* In this example since the length of the transmitted frame does not change,
         * nor the other parameters of the dwt_writetxfctrl function, the
         * dwt_writetxfctrl call could be outside the main while(1) loop.
         */
        dwt_writetxfctrl(FRAME_LENGTH, 0, 0); /* Zero offset in TX buffer, no ranging. */

        /* Start transmission. */
        dwt_starttx(DWT_START_TX_IMMEDIATE);
        /* Poll DW IC until TX frame sent event set. See NOTE 4 below.
         * STATUS register is 4 bytes long but, as the event we are looking
         * at is in the first byte of the register, we can use this simplest
         * API function to access it.*/
        waitforsysstatus(NULL, NULL, DWT_INT_TXFRS_BIT_MASK, 0);

        /* Clear TX frame sent event. */
        dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);

        test_run_info((unsigned char *)"TX Frame Sent \r\n");

        /* Execute a delay between transmissions. */
        Sleep(TX_DELAY_MS);

        /* Increment the blink frame sequence number (modulo 256). */
        tx_msg[BLINK_FRAME_SN_IDX]++;
    }
}
#endif // TEST_TX_POWER_ADJUSTMENT
/*****************************************************************************************************************************************************
 * NOTES:
 * 1. The device ID is a hard coded constant in the blink to keep the example simple but for a real product every device should have a unique ID.
 *    For development purposes it is possible to generate a DW IC unique ID by combining the Lot ID & Part Number values programmed into the
 *    DW IC during its manufacture. However there is no guarantee this will not conflict with someone else's implementation. We recommended that
 *    customers buy a block of addresses from the IEEE Registration Authority for their production items. See "EUI" in the DW IC User Manual.
 * 2. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
 *    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW IC OTP memory.
 * 3. dwt_writetxdata() takes the full size of tx_msg as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
 *    automatically appended by the DW IC. This means that our tx_msg could be two bytes shorter without losing any data (but the sizeof would not
 *    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
 * 4. We use polled mode of operation here to keep the example as simple as possible, but the TXFRS status event can be used to generate an interrupt.
 *    Please refer to DW IC User Manual for more details on "interrupts".
 * 5. Desired configuration by user may be different to the current programmed configuration. dwt_configure is called to set desired
 *    configuration.
 *    For development purposes it is possible to generate a DW IC unique ID by combining the Lot ID & Part Number values programmed into the
 *    DW IC during its manufacture. However there is no guarantee this will not conflict with someone else's implementation. We recommended that
 *    customers buy a block of addresses from the IEEE Registration Authority for their production items. See "EUI" in the DW IC User Manual.
 *    configuration.
 * 6. When manufacturing a product integrating a DW3000, each unit should be calibrated accurately. The TxPower setting for which a unit is passing
 *    regulation should be measured and stored in non-volatile memory. The dwt_adjust_tx_power API allows to calculate a new TxPower setting
 *    relatively to the reference setting measured in factory, and a boost. This can be required when transmitting a shorter frame that can be
 *    transmitted as higher TxPower levels. The 0x36363636 in this example is solely for example purpose. It should not be used in a customer project
 *    or product. The reference setting should be measured/calibrated by the end-customer for its product.
 ****************************************************************************************************************************************************/
