/*! ----------------------------------------------------------------------------
 * @file    example_info.h
 * @brief
 *
 * @attention
 *
 * Copyright 2013-2018(c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#include "examples_defines.h"
#include <assert.h>
#include <example_selection.h>

example_ptr example_pointer;

void build_examples(void)
{
    unsigned char test_cnt = 0;

#ifdef TEST_READING_DEV_ID
    extern int read_dev_id(void);

    example_pointer = read_dev_id;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_TX
    extern int simple_tx(void);

    example_pointer = simple_tx;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_TX_PDOA
    extern int simple_tx_pdoa(void);

    example_pointer = simple_tx_pdoa;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_RX
    extern int simple_rx(void);

    example_pointer = simple_rx;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_RX_NLOS
    extern int simple_rx_nlos(void);

    example_pointer = simple_rx_nlos;
    test_cnt++;
#endif

#ifdef TEST_RX_SNIFF
    extern int rx_sniff(void);

    example_pointer = rx_sniff;
    test_cnt++;
#endif

#ifdef TEST_RX_TRIM
    extern int rx_with_xtal_trim(void);

    example_pointer = rx_with_xtal_trim;
    test_cnt++;
#endif

#ifdef TEST_RX_DIAG
    extern int rx_diagnostics(void);

    example_pointer = rx_diagnostics;
    test_cnt++;
#endif

#ifdef TEST_TX_SLEEP
    extern int tx_sleep(void);

    example_pointer = tx_sleep;
    test_cnt++;
#endif

#ifdef TEST_TX_SLEEP_IDLE_RC
    extern int tx_sleep_idleRC(void);

    example_pointer = tx_sleep_idleRC;
    test_cnt++;
#endif

#ifdef TEST_TX_SLEEP_TIMED
    extern int tx_timed_sleep(void);

    example_pointer = tx_timed_sleep;
    test_cnt++;
#endif

#ifdef TEST_TX_SLEEP_AUTO
    extern int tx_sleep_auto(void);

    example_pointer = tx_sleep_auto;
    test_cnt++;
#endif

#ifdef TEST_TX_WITH_CCA
    extern int tx_with_cca(void);

    example_pointer = tx_with_cca;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_TX_AES
    extern int simple_tx_aes(void);

    example_pointer = simple_tx_aes;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_RX_AES
    extern int simple_rx_aes(void);

    example_pointer = simple_rx_aes;
    test_cnt++;
#endif

#ifdef TEST_TX_WAIT_RESP
    extern int tx_wait_resp(void);

    example_pointer = tx_wait_resp;
    test_cnt++;
#endif

#ifdef TEST_TX_WAIT_RESP_INT
    extern int tx_wait_resp_int(void);

    example_pointer = tx_wait_resp_int;
    test_cnt++;
#endif

#ifdef TEST_RX_SEND_RESP
    extern int rx_send_resp(void);

    example_pointer = rx_send_resp;
    test_cnt++;
#endif

#ifdef TEST_SS_TWR_RESPONDER
    extern int ss_twr_responder(void);

    example_pointer = ss_twr_responder;
    test_cnt++;
#endif

#ifdef TEST_SS_TWR_INITIATOR
    extern int ss_twr_initiator(void);

    example_pointer = ss_twr_initiator;
    test_cnt++;
#endif

#ifdef TEST_SS_TWR_INITIATOR_STS
    extern int ss_twr_initiator_sts(void);

    example_pointer = ss_twr_initiator_sts;
    test_cnt++;
#endif

#ifdef TEST_SS_TWR_RESPONDER_STS
    extern int ss_twr_responder_sts(void);

    example_pointer = ss_twr_responder_sts;
    test_cnt++;
#endif

#ifdef TEST_SS_TWR_INITIATOR_STS_NO_DATA
    extern int ss_twr_initiator_sts_no_data(void);

    example_pointer = ss_twr_initiator_sts_no_data;
    test_cnt++;
#endif

#ifdef TEST_SS_TWR_RESPONDER_STS_NO_DATA
    extern int ss_twr_responder_sts_no_data(void);

    example_pointer = ss_twr_responder_sts_no_data;
    test_cnt++;
#endif

#ifdef TX_RX_AES_VERIFICATION
    extern int tx_rx_aes_verification(void);

    example_pointer = tx_rx_aes_verification;
    test_cnt++;
#endif

#ifdef TEST_AES_SS_TWR_INITIATOR
    extern int ss_aes_twr_initiator(void);

    example_pointer = ss_aes_twr_initiator;
    test_cnt++;
#endif

#ifdef TEST_AES_SS_TWR_RESPONDER
    extern int ss_aes_twr_responder(void);

    example_pointer = ss_aes_twr_responder;
    test_cnt++;
#endif

#ifdef TEST_DS_TWR_INITIATOR
    extern int ds_twr_initiator(void);

    example_pointer = ds_twr_initiator;
    test_cnt++;
#endif

#ifdef TEST_DS_TWR_RESPONDER
    extern int ds_twr_responder(void);

    example_pointer = ds_twr_responder;
    test_cnt++;
#endif

#ifdef TEST_DS_TWR_RESPONDER_STS
    extern int ds_twr_responder_sts(void);

    example_pointer = ds_twr_responder_sts;
    test_cnt++;
#endif

#ifdef TEST_DS_TWR_INITIATOR_STS
    extern int ds_twr_initiator_sts(void);

    example_pointer = ds_twr_initiator_sts;
    test_cnt++;
#endif

#ifdef TEST_DS_TWR_STS_SDC_INITIATOR
    extern int ds_twr_sts_sdc_initiator(void);

    example_pointer = ds_twr_sts_sdc_initiator;
    test_cnt++;
#endif

#ifdef TEST_DS_TWR_STS_SDC_RESPONDER
    extern int ds_twr_sts_sdc_responder(void);

    example_pointer = ds_twr_sts_sdc_responder;
    test_cnt++;
#endif

#ifdef TEST_CONTINUOUS_WAVE
    extern int continuous_wave_example(void);

    example_pointer = continuous_wave_example;
    test_cnt++;
#endif

#ifdef TEST_CONTINUOUS_FRAME
    extern int continuous_frame_example(void);

    example_pointer = continuous_frame_example;
    test_cnt++;

#endif

#ifdef TEST_ACK_DATA_RX
    extern int ack_data_rx(void);

    example_pointer = ack_data_rx;
    test_cnt++;

#endif

#ifdef TEST_ACK_DATA_TX
    extern int ack_data_tx(void);

    example_pointer = ack_data_tx;
    test_cnt++;

#endif

#ifdef TEST_GPIO
    extern int gpio_example(void);

    example_pointer = gpio_example;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_TX_STS_SDC
    extern int simple_tx_sts_sdc(void);

    example_pointer = simple_tx_sts_sdc;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_RX_STS_SDC
    extern int simple_rx_sts_sdc(void);

    example_pointer = simple_rx_sts_sdc;
    test_cnt++;
#endif

#ifdef TEST_FRAME_FILTERING_TX
    extern int frame_filtering_tx(void);

    example_pointer = frame_filtering_tx;
    test_cnt++;
#endif

#ifdef TEST_FRAME_FILTERING_RX
    extern int frame_filtering_rx(void);

    example_pointer = frame_filtering_rx;
    test_cnt++;
#endif

#ifdef TEST_SPI_CRC
    extern int spi_crc(void);

    example_pointer = spi_crc;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_RX_PDOA
    extern int simple_rx_pdoa(void);

    example_pointer = simple_rx_pdoa;
    test_cnt++;
#endif

#ifdef TEST_OTP_WRITE
    extern int otp_write(void);

    example_pointer = otp_write;
    test_cnt++;
#endif

#ifdef TEST_LE_PEND_TX
    extern int le_pend_tx(void);

    example_pointer = le_pend_tx;
    test_cnt++;
#endif

#ifdef TEST_LE_PEND_RX
    extern int le_pend_rx(void);

    example_pointer = le_pend_rx;
    test_cnt++;
#endif

#ifdef TEST_PLL_CAL
    extern int pll_cal(void);

    example_pointer = pll_cal;
    test_cnt++;
#endif

#ifdef TEST_BW_CAL
    extern int bw_cal(void);

    example_pointer = bw_cal;
    test_cnt++;
#endif

#ifdef TEST_DOUBLE_BUFFER_RX
    extern int double_buffer_rx(void);

    example_pointer = double_buffer_rx;
    test_cnt++;
#endif

#ifdef TEST_TIMER
    extern int timer_example(void);

    example_pointer = timer_example;
    test_cnt++;
#endif

#ifdef TEST_TX_POWER_ADJUSTMENT
    extern int tx_power_adjustment_example(void);

    example_pointer = tx_power_adjustment_example;
    test_cnt++;
#endif

#ifdef TEST_SIMPLE_AES
    extern int simple_aes(void);

    example_pointer = simple_aes;
    test_cnt++;
#endif
    // Check that only 1 test was enabled in test_selection.h file
    assert(test_cnt == 1);
}
