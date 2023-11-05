/*! ----------------------------------------------------------------------------
 * @file    deca_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "deca_spi.h"
#include "port.h"
#include <deca_device_api.h>

static spi_handle_t spi_handler;
static spi_handle_t *pgSpiHandler = &spi_handler;

uint16_t  current_cs_pin=DW3000_CS_Pin;
uint16_t  current_irq_pin=DW3000_IRQn_Pin;

dw_t s
=
{
    .irqPin    = DW3000_IRQn_Pin,
    .rstPin    = DW3000_RESET_Pin,
    .wkupPin   = DW3000_WAKEUP_Pin,
    .csPin     = DW3000_CS_Pin,    //'1' steady state
    .pSpi      = &spi_handler,
};

const dw_t *SPI = &s;

static volatile bool spi_xfer_done;
static uint8_t spi_init_stat = 0; // use 1 for slow, use 2 for fast;

static uint8_t idatabuf[DATALEN1] = { 0 }; // Never define this inside the Spi read/write
static uint8_t itempbuf[DATALEN1] = { 0 }; // As that will use the stack from the Task, which are not such long!!!!
                                           // You will face a crashes which are not expected!

/****************************************************************************
 *
 *                              DW3000 SPI section
 *
 *******************************************************************************/

/* @fn    nrf52840_dk_spi_init
 * Initialise nRF52840-DK SPI
 * */
void nrf52840_dk_spi_init(void)
{
    nrf_drv_spi_t *spi_inst;
    nrf_drv_spi_config_t *spi_config;

    spi_handle_t *pSPI_handler = SPI->pSpi;

    pSPI_handler->frequency_slow = NRF_SPIM_FREQ_4M;
    pSPI_handler->frequency_fast = NRF_SPIM_FREQ_32M;

    pSPI_handler->lock = DW_HAL_NODE_UNLOCKED;

    spi_inst = &pSPI_handler->spi_inst;
    spi_config = &pSPI_handler->spi_config;

    spi_inst->inst_idx = SPI3_INSTANCE_INDEX;
    spi_inst->use_easy_dma = SPI3_USE_EASY_DMA;
    spi_inst->u.spim.p_reg = NRF_SPIM3;
    spi_inst->u.spim.drv_inst_idx = NRFX_SPIM3_INST_IDX;

    spi_config->sck_pin = DW3000_CLK_Pin;
    spi_config->mosi_pin = DW3000_MOSI_Pin;
    spi_config->miso_pin = DW3000_MISO_Pin;
    spi_config->ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config->irq_priority = (APP_IRQ_PRIORITY_MID - 2);
    spi_config->orc = 0xFF;
    spi_config->frequency = NRF_SPIM_FREQ_4M;
    spi_config->mode = NRF_DRV_SPI_MODE_0;
    spi_config->bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

    // Configure the chip select as an output pin that can be toggled
    nrf_drv_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_TASK_TOGGLE(NRF_GPIOTE_INITIAL_VALUE_HIGH);
    nrf_drv_gpiote_out_init(DW3000_CS_Pin, &out_config);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
static int openspi(nrf_drv_spi_t *p_instance)
{
    NRF_SPIM_Type *p_spi = p_instance->u.spim.p_reg;
    nrf_spim_enable(p_spi);
    return 0;
} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
static int closespi(nrf_drv_spi_t *p_instance)
{
    NRF_SPIM_Type *p_spi = p_instance->u.spim.p_reg;
    nrf_spim_disable(p_spi);
    return 0;
} // end closespi()

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context)
{
    UNUSED_PARAMETER(p_event);
    UNUSED_PARAMETER(p_context);
    spi_xfer_done = true;
}

/* @fn      port_set_dw_ic_spi_slowrate
 * @brief   set 4MHz
 * */
void port_set_dw_ic_spi_slowrate(void)
{

    pgSpiHandler->spi_config.frequency = pgSpiHandler->frequency_slow;

    APP_ERROR_CHECK(nrf_drv_spi_init(&pgSpiHandler->spi_inst,
                                     &pgSpiHandler->spi_config,
                                     NULL,
                                     NULL) );


    nrf_delay_ms(2);

}

/* @fn      port_set_dw_ic_spi_fastrate
 * @brief   set 16MHz for SPI_1 and 8MHz for SPI_2
 * */
void port_set_dw_ic_spi_fastrate(void)
{

    pgSpiHandler->spi_config.frequency = pgSpiHandler->frequency_fast;

    APP_ERROR_CHECK( nrf_drv_spi_init(&pgSpiHandler->spi_inst,
                                      &pgSpiHandler->spi_config,
                                      NULL,
                                      NULL) );

    nrf_gpio_cfg(pgSpiHandler->spi_config.sck_pin,
                     NRF_GPIO_PIN_DIR_OUTPUT,
                     NRF_GPIO_PIN_INPUT_CONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_H0H1,
                     NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg( pgSpiHandler->spi_config.mosi_pin,
                     NRF_GPIO_PIN_DIR_OUTPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_H0H1,
                     NRF_GPIO_PIN_NOSENSE);

    nrf_delay_ms(2);

}

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospiwithcrc()
 *
 * Low level abstract function to write to the SPI when SPI CRC mode is used
 * Takes two separate byte buffers for write header and write data, and a CRC8 byte which is written last
 * returns 0 for success, or -1 for error
 */
int writetospiwithcrc(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer, uint8_t crc8)
{
#ifdef DWT_ENABLE_CRC
    uint8_t *p1;
    uint32_t idatalength = headerLength + bodyLength + sizeof(crc8); // It cannot be more than 255 in total length (header + body)

    if (idatalength > DATALEN1)
    {
        return NRF_ERROR_NO_MEM;
    }

    while(pgSpiHandler->lock);

    __HAL_LOCK(pgSpiHandler);

    openspi(&pgSpiHandler->spi_inst);

    p1 = idatabuf;
    memcpy(p1, headerBuffer, headerLength);
    p1 += headerLength;
    memcpy(p1, bodyBuffer, bodyLength);
    p1 += bodyLength;
    memcpy(p1, &crc8, 1);

    nrfx_gpiote_out_toggle(current_cs_pin);

    spi_xfer_done = false;
    nrf_drv_spi_transfer(&pgSpiHandler->spi_inst, idatabuf, idatalength, itempbuf, idatalength);

    closespi(&pgSpiHandler->spi_inst);
    nrfx_gpiote_out_toggle(current_cs_pin);

    __HAL_UNLOCK(pgSpiHandler);
#endif //DWT_ENABLE_CRC
    return 0;
} // end writetospiwithcrc()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer)
{
    uint8_t *p1;
    uint32_t idatalength = headerLength + bodyLength;

    if (idatalength > DATALEN1)
    {
        return NRF_ERROR_NO_MEM;
    }

    while(pgSpiHandler->lock);

    __HAL_LOCK(pgSpiHandler);

    openspi(&pgSpiHandler->spi_inst);

    p1 = idatabuf;
    memcpy(p1, headerBuffer, headerLength);
    p1 += headerLength;
    memcpy(p1, bodyBuffer, bodyLength);

    nrfx_gpiote_out_toggle(current_cs_pin);

    spi_xfer_done = false;
    nrf_drv_spi_transfer(&pgSpiHandler->spi_inst, idatabuf, idatalength, itempbuf, idatalength);

    closespi(&pgSpiHandler->spi_inst);
    nrfx_gpiote_out_toggle(current_cs_pin);
     __HAL_UNLOCK(pgSpiHandler);

    return 0;
} // end writetospi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readLength, uint8_t *readBuffer)
{
    uint8_t *p1;
    uint32_t idatalength = headerLength + readLength;

    if (idatalength > DATALEN1)
    {
        return NRF_ERROR_NO_MEM;
    }

    while(pgSpiHandler->lock);

    __HAL_LOCK(pgSpiHandler);

    openspi(&pgSpiHandler->spi_inst);

    p1 = idatabuf;
    memcpy(p1, headerBuffer, headerLength);

    p1 += headerLength;
    memset(p1, 0x00, readLength);

    idatalength = headerLength + readLength;

    nrfx_gpiote_out_toggle(current_cs_pin);

    spi_xfer_done = false;
    nrf_drv_spi_transfer(&pgSpiHandler->spi_inst, idatabuf, idatalength, itempbuf, idatalength);

    p1 = itempbuf + headerLength;
    memcpy(readBuffer, p1, readLength);

    closespi(&pgSpiHandler->spi_inst);
    nrfx_gpiote_out_toggle(current_cs_pin);

    __HAL_UNLOCK(pgSpiHandler);

    return 0;
} // end readfromspi()

/****************************************************************************
 *
 *                              END OF DW3000 SPI section
 *
 *******************************************************************************/
