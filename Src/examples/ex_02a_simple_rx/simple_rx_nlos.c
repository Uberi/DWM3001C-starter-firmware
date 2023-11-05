/*! ----------------------------------------------------------------------------
 *  @file    simple_rx_nlos.c
 *  @brief   Simple RX NLOS example code
 *           This is a simple code example that turns on the DW IC receiver to receive a frame, (expecting the frame as sent by the companion simple
 *           example "Simple TX example code"). When a frame is received and validated based on the diagnostics logged, diagnostic register values
 *           are read and calculations for First Path Power based on the section 4.7.1 and Estimating the receive signal power based on 4.7.2 of the
 *           User Manual. The probability of signal being Line of Sight or Non-Line of Sight is calculated based on the App Notes "APS006 PART 3.
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
#include <math.h>
#include <port.h>
#include <shared_defines.h>
#include <shared_functions.h>
#include <string.h>

#if defined(TEST_SIMPLE_RX_NLOS)

extern void test_run_info(unsigned char *data);

/* Example application name */
#define APP_NAME "SIMPLE RX_NLOS v1.0"

#define SIG_LVL_FACTOR     0.4 // Factor between 0 and 1; default 0.4 from experiments and simulations.
#define SIG_LVL_THRESHOLD  12 // Threshold unit is dB; default 12dB from experiments and simulations.
#define ALPHA_PRF_16       113.8 // Constant A for PRF of 16 MHz. See User Manual for more information.
#define ALPHA_PRF_64       120.7 // Constant A for PRF of 64 MHz. See User Manual for more information.
#define RX_CODE_THRESHOLD  8 // For 64 MHz PRF the RX code is 9.
#define LOG_CONSTANT_C0    63.2 // 10log10(2^21) = 63.2    // See User Manual for more information.
#define LOG_CONSTANT_D0_E0 51.175 // 10log10(2^17) = 51.175  // See User Manual for more information.
#define IP_MIN_THRESHOLD   3.3 // Minimum Signal Level in dB. Please see App Notes "APS006 PART 3"
#define IP_MAX_THRESHOLD   6.0 // Minimum Signal Level in dB. Please see App Notes "APS006 PART 3"
#define CONSTANT_PR_IP_A   0.39178 // Constant from simulations on DW device accumulator, please see App Notes "APS006 PART 3"
#define CONSTANT_PR_IP_B   1.31719 // Constant from simulations on DW device accumulator, please see App Notes "APS006 PART 3"

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
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_128,  /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

/* Buffer to store received frame. See NOTE 1 below. */
static uint8_t rx_buffer[FRAME_LEN_MAX];

char prob_str[30] = { 0 };

/**
 * Application entry point.
 */
int simple_rx_nlos(void)
{
    /* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
    uint32_t status_reg = 0;
    /* Hold copy of frame length of frame received (if good) so that it can be examined at a debug breakpoint. */
    uint16_t frame_len = 0;

    /* Line-of-sight / Non-line-of-sight Variables */
    uint32_t dev_id;
    uint8_t D;
    dwt_nlos_alldiag_t all_diag;
    dwt_nlos_ipdiag_t index;

    // All float variables used for recording different diagnostic results and probability.
    float ip_f1, ip_f2, ip_f3, sts1_f1, sts1_f2, sts1_f3, sts2_f1, sts2_f2, sts2_f3 = 0;
    float ip_n, sts1_n, sts2_n, ip_cp, sts1_cp, sts2_cp, ip_rsl, ip_fsl, sts1_rsl, sts1_fsl, sts2_rsl, sts2_fsl = 0;
    float pr_nlos, sl_diff_ip, sl_diff_sts1, sl_diff_sts2, sl_diff, index_diff = 0;
    float alpha, ip_alpha, log_constant = 0;

    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* Configure SPI rate, DW IC supports up to 36 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    dev_id = dwt_readdevid();

    while (!dwt_checkidlerc()) /* Need to make sure DW IC is in IDLE_RC before proceeding */ { };

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED");
        while (1) { };
    }

    /* Enabling LEDs here for debug so that for each RX-enable the D2 LED will flash on DW3000 red eval-shield boards. */
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

    /* Configure DW IC. */
    /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
    if (dwt_configure(&config))
    {
        test_run_info((unsigned char *)"CONFIG FAILED     ");
        while (1) { };
    }

    dwt_configciadiag(DW_CIA_DIAG_LOG_ALL);
    /* Loop forever receiving frames. */
    while (TRUE)
    {
        /* TESTING BREAKPOINT LOCATION #1 */

        /* Clear local RX buffer to avoid having leftovers from previous receptions  This is not necessary but is included here to aid reading
         * the RX buffer.
         * This is a good place to put a breakpoint. Here (after first time through the loop) the local status register will be set for last event
         * and if a good receive has happened the data buffer will have the data in it, and frame_len will be set to the length of the RX frame. */
        memset(rx_buffer, 0, sizeof(rx_buffer));

        /* Activate reception immediately. See NOTE 2 below. */
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        /* Poll until a frame is properly received or an error/timeout occurs. See NOTE 3 below.
         * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
         * function to access it. */
        waitforsysstatus(&status_reg, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR), 0);

        // Check if the received frame is good.
        if (status_reg & DWT_INT_RXFCG_BIT_MASK)
        {
            /* A frame has been received, copy it to our local buffer. */
            frame_len = dwt_getframelength();
            if (frame_len <= FRAME_LEN_MAX)
            {
                dwt_readrxdata(rx_buffer, frame_len - FCS_LEN, 0); /* No need to read the FCS/CRC. */
            }

            /* Clear good RX frame event in the DW IC status register. */
            dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);

            test_run_info((unsigned char *)"Frame Received");

            if ((dev_id == (uint32_t)DWT_DW3000_DEV_ID) || (dev_id == (uint32_t)DWT_DW3000_PDOA_DEV_ID))
            {
                log_constant = LOG_CONSTANT_C0;
            }
            else
            {
                log_constant = LOG_CONSTANT_D0_E0;
            }

            // Select IPATOV to read Ipatov diagnostic registers from API function dwt_nlos_alldiag()
            all_diag.diag_type = IPATOV;
            dwt_nlos_alldiag(&all_diag);
            ip_alpha = (config.rxCode > RX_CODE_THRESHOLD) ? (-(ALPHA_PRF_64 + 1)) : -(ALPHA_PRF_16);
            ip_n = all_diag.accumCount; // The number of preamble symbols accumulated
            ip_f1 = all_diag.F1 / 4;    // The First Path Amplitude (point 1) magnitude value (it has 2 fractional bits),
            ip_f2 = all_diag.F2 / 4;    // The First Path Amplitude (point 2) magnitude value (it has 2 fractional bits),
            ip_f3 = all_diag.F3 / 4;    // The First Path Amplitude (point 3) magnitude value (it has 2 fractional bits),
            ip_cp = all_diag.cir_power;

            // Select STS1 to read STS1 diagnostic registers from API function dwt_nlos_alldiag()
            all_diag.diag_type = STS1;
            dwt_nlos_alldiag(&all_diag);
            alpha = -(ALPHA_PRF_64 + 1);
            sts1_n = all_diag.accumCount; // The number of preamble symbols accumulated
            sts1_f1 = all_diag.F1 / 4;    // The First Path Amplitude (point 1) magnitude value (it has 2 fractional bits),
            sts1_f2 = all_diag.F2 / 4;    // The First Path Amplitude (point 2) magnitude value (it has 2 fractional bits),
            sts1_f3 = all_diag.F3 / 4;    // The First Path Amplitude (point 3) magnitude value (it has 2 fractional bits),
            sts1_cp = all_diag.cir_power;

            // Select STS2 to read STS2 diagnostic registers from API function dwt_nlos_alldiag()
            all_diag.diag_type = STS2;
            dwt_nlos_alldiag(&all_diag);
            sts2_n = all_diag.accumCount; // The number of preamble symbols accumulated
            sts2_f1 = all_diag.F1 / 4;    // The First Path Amplitude (point 1) magnitude value (it has 2 fractional bits),
            sts2_f2 = all_diag.F2 / 4;    // The First Path Amplitude (point 2) magnitude value (it has 2 fractional bits),
            sts2_f3 = all_diag.F3 / 4;    // The First Path Amplitude (point 3) magnitude value (it has 2 fractional bits),
            sts2_cp = all_diag.cir_power;

            // The calculation of First Path Power Level(FSL) and Receive Signal Power Level(RSL) is taken from
            // DW3000 User Manual section 4.7.1 & 4.7.2

            // For IPATOV
            ip_n *= ip_n;
            ip_f1 *= ip_f1;
            ip_f2 *= ip_f2;
            ip_f3 *= ip_f3;

            // FOR STS1
            sts1_n *= sts1_n;
            sts1_f1 *= sts1_f1;
            sts1_f2 *= sts1_f2;
            sts1_f3 *= sts1_f3;

            // For STS2
            sts2_n *= sts2_n;
            sts2_f1 *= sts2_f1;
            sts2_f2 *= sts2_f2;
            sts2_f3 *= sts2_f3;

            D = all_diag.D * 6;

            // Calculate the First Signal Level(FSL) and Receive Signal Level(RSL) then subtract FSL from RSL
            // to find out Signal Level Difference which is compared to defined Signal Threshold.

            // For the CIR Ipatov.
            ip_rsl = 10 * log10((float)ip_cp / ip_n) + ip_alpha + log_constant + D;
            ip_fsl = 10 * log10(((ip_f1 + ip_f2 + ip_f3) / ip_n)) + ip_alpha + D;

            // Signal Level Difference value for IPATOV.
            sl_diff_ip = ip_rsl - ip_fsl;

            // For the CIR STS1.
            sts1_rsl = 10 * log10((float)sts1_cp / sts1_n) + alpha + log_constant + D;
            sts1_fsl = 10 * log10(((sts1_f1 + sts1_f2 + sts1_f3) / sts1_n)) + alpha + D;

            // For the CIR STS2.
            sts2_rsl = 10 * log10((float)sts2_cp / sts2_n) + alpha + log_constant + D;
            sts2_fsl = 10 * log10(((sts2_f1 + sts2_f2 + sts2_f3) / sts2_n)) + alpha + D;

            // STS Mode OFF, Signal Level Difference of STS1 and STS2 is zero.
            if (config.stsMode == DWT_STS_MODE_OFF)
            {
                sl_diff_sts1 = 0;
                sl_diff_sts2 = 0;
            }
            else // If PDOA MODE 3 is enabled then there's Signal Level Difference value for all IPATOV, STS1 and STS2.
            {
                sl_diff_sts1 = sts1_rsl - sts1_fsl;

                // IF PDOA MODE 3 is not enabled then Signal Level Difference of STS2 is zero.
                if (config.pdoaMode != DWT_PDOA_M3)
                {
                    sl_diff_sts2 = 0;
                }
                else
                {
                    sl_diff_sts2 = sts2_rsl - sts2_fsl;
                }
            }

            /* Check for Line-of-sight or Non-line-of-sight */
            // The Signal Level Threshold is 12 dB, based on the experiments and simulations if the received signal power is above
            // 12 dB then the Signal is Non Line of Sight.

            // Calculating the probability of NLOS.
            // 1. If the signal level difference of IPATOV, STS1 or STS2 is greater than 12 dB then the signal is Non Line of sight.

            if ((sl_diff_ip > SIG_LVL_THRESHOLD) || (sl_diff_sts1 > SIG_LVL_THRESHOLD) || (sl_diff_sts2 > SIG_LVL_THRESHOLD))
            {
                pr_nlos = 100;
                test_run_info((unsigned char *)"Non-Line of sight");
            }

            // If the received signal power is in between 4.8 dB and 12 dB then there's a possibility that the signal is
            // Non Line of Sight and probabilty of it being Non Line of Sight signal is calculated.

            // 2. If the signal level difference of IPATOV, STS1 or STS2 is greater than
            //    (Signal Level Threshold(12) * Signal Level Factor(0.4)) = 4.8 dB but less than 12 dB, then calculate the
            //    probability of Non Line of sight based on the signal has greater strength(IPATOV, STS1 or STS2).

            else if ((sl_diff_ip > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR) || (sl_diff_sts1 > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR)
                     || (sl_diff_sts2 > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR))
            {
                if (sl_diff_ip > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR)
                {
                    sl_diff = sl_diff_ip;
                }
                else if (sl_diff_sts1 > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR)
                {
                    sl_diff = sl_diff_sts1;
                }
                else
                {
                    sl_diff = sl_diff_sts2;
                }

                pr_nlos = 100 * ((sl_diff / SIG_LVL_THRESHOLD - SIG_LVL_FACTOR) / (1 - SIG_LVL_FACTOR));
                snprintf(prob_str, sizeof(prob_str), "Probability of NLOS: %3.2f", fabsf(pr_nlos));
                test_run_info((unsigned char *)prob_str);
            }

            // 3. If the signal is less than the Combined Threshold (Signal Level Threshold(12) * Signal Level Factor(0.4)) for all three
            //    IPATOV, STS1 and STS2 reported through dwt_nlos_alldiag() then check the IPATOV Diagnostic First Path and Peak Path Index
            //    through dwt_nlos_ipdiag().

            //    3.a. If the Index difference is less than 3.3 dB then it's a Line of Sight signal.
            //    3.b. If the Index difference is greater than 3.3 dB and less than 6 dB then the probability of Non Line of Sight
            //         is calculated.
            //    3.c. If the Index level is greater than 6 dB then it's a Non Line of Sight signal.
            else
            {
                dwt_nlos_ipdiag(&index);
                index_diff = ((float)index.index_pp_u32 - (float)index.index_fp_u32) / 32;

                if (index_diff <= IP_MIN_THRESHOLD)
                {
                    pr_nlos = 0;
                    test_run_info((unsigned char *)"Line of Sight");
                }
                else if ((index_diff > IP_MIN_THRESHOLD) && (index_diff < IP_MAX_THRESHOLD))
                {
                    pr_nlos = 100 * ((CONSTANT_PR_IP_A * index_diff) - CONSTANT_PR_IP_B);
                    snprintf(prob_str, sizeof(prob_str), "**Probability of NLOS: %3.2f", fabsf(pr_nlos));
                    test_run_info((unsigned char *)prob_str);
                }
                else
                {
                    pr_nlos = 100;
                    test_run_info((unsigned char *)"Non-Line of Sight");
                }
            }
        }
        else
        {
            /* Clear RX error events in the DW IC status register. */
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_ERR);
        }
    }
}

#endif

/*****************************************************************************************************************************************************
 * NOTES:
 * 1.  In this example, maximum frame length is set to 127 bytes which is 802.15.4 UWB standard maximum frame length. DW IC supports an extended
 *     frame length (up to 1023 bytes long) mode which is not used in this example.
 * 2.  Manual reception activation is performed here but DW IC offers several features that can be used to handle more complex scenarios or to
 *     optimise system's overall performance (e.g. timeout after a given time, automatic re-enabling of reception in case of errors, etc.).
 * 3.  We use polled mode of operation here to keep the example as simple as possible, but RXFCG and error/timeout status events can be used to
 *     generate interrupts. Please refer to DW IC User Manual for more details on "interrupts".
 * 4.  Enable the CIA Diagnostics "dwt_configciadiag()" before RX ENABLE.
 *
 * Please see defines at the beginning of the file for detailed explanation of the threshold values.
 * 5.  The Signal Level Threshold is 12 dB and Signal Level Factor 0.4 dB.
 * 6.  If PDOA MODE 3 is enabled then all three IPATOV, STS1 and STS2 will report a Signal Level difference.
 * 7.  If the signal level difference of IPATOV, STS1 or STS2 is greater than 12 dB then the signal is Non Line of sight.
 * 8.  If the signal level difference of IPATOV, STS1 or STS2 is greater than Signal Level Threshold * Signal Level Factor(12*0.4) = 4.8 dB then
       calculate the "Probability of Non Line of Sight" based on the signal which has a greater signal level difference(IPATOV, STS1 or STS2).
 * 9.  If the signal is less than the Combined Threshold (Signal Level Threshold * Signal Level Factor) for all three - IPATOV, STS1 and STS2
       reported through "dwt_nlos_alldiag()" then check the IPATOV Diagnostic First Path and Peak Path Index through "dwt_nlos_ipdiag()".
 * 10. When STS is OFF and Index difference is less than 3.3 dB then it's a Line of Sight signal. See App Notes "APS006 PART 3"
 * 11. When STS is OFF and the Index difference is greater than 3.3 dB and less than 6 dB then the probability of Non Line of Sight
       is calculated. See App Notes "APS006 PART 3"
 * 12. When STS is OFF and the Index level is greater than 6 dB then it's a Non Line of Sight signal. See App Notes "APS006 PART 3"
 ****************************************************************************************************************************************************/
