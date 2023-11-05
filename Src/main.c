/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 2014 - 2020 SEGGER Microcontroller GmbH             *
*                                                                    *
*           www.segger.com     Support: support@segger.com           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* conditions are met:                                                *
*                                                                    *
* - Redistributions of source code must retain the above copyright   *
*   notice, this list of conditions and the following disclaimer.    *
*                                                                    *
* - Neither the name of SEGGER Microcontroller GmbH                  *
*   nor the names of its contributors may be used to endorse or      *
*   promote products derived from this software without specific     *
*   prior written permission.                                        *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED.                                                        *
* IN NO EVENT SHALL SEGGER Microcontroller GmbH BE LIABLE FOR        *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : DWM3001C build main entry point for simple exmaples.

*/

#include <boards.h>
#include <deca_spi.h>
#include <port.h>
#include <sdk_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn test_run_info()
 *
 * @brief  This function is simply a printf() call for a string. It is implemented differently on other platforms,
 *         but on the DWM3001C, a printf() call is .
 *
 * @param data - Message data, this data should be NULL string.
 *
 * output parameters
 *
 * no return value
 */
void test_run_info(unsigned char *data)
{
    printf("%s\n", data);
}

int main(void)
{
    /* Initialize all configured peripherals */
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);

    /* Initialise DWM3001C GPIOs */
    gpio_init();

    /* Initialise the SPI for DWM3001C */
    dwm3001c_spi_init();

    /* Configuring interrupt*/
    dw_irq_init();

    /* Small pause before startup */
    nrf_delay_ms(2);

    // UNCOMMENT EXACTLY ONE OF THE BELOW EXAMPLES, AND ALSO UNCOMMENT THE CORRESPONDING #define IN example_selection.h:
    extern int read_dev_id(void); read_dev_id();
    // extern int simple_tx(void); simple_tx();
    // extern int simple_tx_pdoa(void); simple_tx_pdoa();
    // extern int simple_rx(void); simple_rx();
    // extern int simple_rx_nlos(void); simple_rx_nlos();
    // extern int rx_sniff(void); rx_sniff();
    // extern int rx_with_xtal_trim(void); rx_with_xtal_trim();
    // extern int rx_diagnostics(void); rx_diagnostics();
    // extern int tx_sleep(void); tx_sleep();
    // extern int tx_sleep_idleRC(void); tx_sleep_idleRC();
    // extern int tx_timed_sleep(void); tx_timed_sleep();
    // extern int tx_sleep_auto(void); tx_sleep_auto();
    // extern int tx_with_cca(void); tx_with_cca();
    // extern int simple_tx_aes(void); simple_tx_aes();
    // extern int simple_rx_aes(void); simple_rx_aes();
    // extern int tx_wait_resp(void); tx_wait_resp();
    // extern int tx_wait_resp_int(void); tx_wait_resp_int();
    // extern int rx_send_resp(void); rx_send_resp();
    // extern int ss_twr_responder(void); ss_twr_responder();
    // extern int ss_twr_initiator(void); ss_twr_initiator();
    // extern int ss_twr_initiator_sts(void); ss_twr_initiator_sts();
    // extern int ss_twr_responder_sts(void); ss_twr_responder_sts();
    // extern int ss_twr_initiator_sts_no_data(void); ss_twr_initiator_sts_no_data();
    // extern int ss_twr_responder_sts_no_data(void); ss_twr_responder_sts_no_data();
    // extern int tx_rx_aes_verification(void); tx_rx_aes_verification();
    // extern int ss_aes_twr_initiator(void); ss_aes_twr_initiator();
    // extern int ss_aes_twr_responder(void); ss_aes_twr_responder();
    // extern int ds_twr_initiator(void); ds_twr_initiator();
    // extern int ds_twr_responder(void); ds_twr_responder();
    // extern int ds_twr_responder_sts(void); ds_twr_responder_sts();
    // extern int ds_twr_initiator_sts(void); ds_twr_initiator_sts();
    // extern int ds_twr_sts_sdc_initiator(void); ds_twr_sts_sdc_initiator();
    // extern int ds_twr_sts_sdc_responder(void); ds_twr_sts_sdc_responder();
    // extern int continuous_wave_example(void); continuous_wave_example();
    // extern int continuous_frame_example(void); continuous_frame_example();
    // extern int ack_data_rx(void); ack_data_rx();
    // extern int ack_data_tx(void); ack_data_tx();
    // extern int gpio_example(void); gpio_example();
    // extern int simple_tx_sts_sdc(void); simple_tx_sts_sdc();
    // extern int simple_rx_sts_sdc(void); simple_rx_sts_sdc();
    // extern int frame_filtering_tx(void); frame_filtering_tx();
    // extern int frame_filtering_rx(void); frame_filtering_rx();
    // extern int spi_crc(void); spi_crc();
    // extern int simple_rx_pdoa(void); simple_rx_pdoa();
    // extern int otp_write(void); otp_write();
    // extern int le_pend_tx(void); le_pend_tx();
    // extern int le_pend_rx(void); le_pend_rx();
    // extern int pll_cal(void); pll_cal();
    // extern int bw_cal(void); bw_cal();
    // extern int double_buffer_rx(void); double_buffer_rx();
    // extern int timer_example(void); timer_example();
    // extern int tx_power_adjustment_example(void); tx_power_adjustment_example();
    // extern int simple_aes(void); simple_aes();

    while (1) {}
}
