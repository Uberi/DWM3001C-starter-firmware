/*! ----------------------------------------------------------------------------
 * @file    port.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2016 - 2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "port.h"
extern uint16_t  current_irq_pin;
/****************************************************************************//**
 *
 *                              APP global variables
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                  Port private variables and function prototypes
 *
 *******************************************************************************/
/* DW IC IRQ handler definition. */
static port_dwic_isr_t port_dwic_isr = NULL;

/****************************************************************************
 *
 *                              Time section
 *
 *******************************************************************************/

/* @fn    Sleep
 * @brief Sleep delay in ms using SysTick timer
 * */
__INLINE void Sleep(uint32_t x)
{
    nrf_delay_ms(x);
}

/****************************************************************************
 *
 *                              END OF Time section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                              Configuration section
 *
 *******************************************************************************/

/* @fn    peripherals_init
 * No perifpherals used in this port.
 * */
int peripherals_init(void)
{
    return 0;
}

/* @fn    gpio_init
 * @brief Initialises the GPIOs of nRF52840-DK board
 * */
void gpio_init(void)
{
    ret_code_t err_code;
    err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);
}

/* @fn    deca_irq_handler
 * @brief handler to invoke the interrupt for call back function.
 * */
void deca_irq_handler(nrf_drv_gpiote_pin_t irqPin, nrf_gpiote_polarity_t irq_action)
{
    process_deca_irq();
}

/* @fn    deca_irq_handler
 * @brief Configures the interrupt. Select the right respective I/O pin and disables it.
 * */
void dw_irq_init(void)
{
    ret_code_t err_code;

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;

    err_code = nrf_drv_gpiote_in_init(DW3000_IRQn_Pin, &in_config, deca_irq_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(DW3000_IRQn_Pin, false);

    nrf_gpio_cfg_output(DW3000_WAKEUP_Pin);

    err_code = nrf_drv_gpiote_in_init(DW3000_IRQ2n_Pin, &in_config, deca_irq_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(DW3000_IRQ2n_Pin, false);

}

/****************************************************************************
 *
 *                          End of configuration section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                          DW IC port section
 *
 *******************************************************************************/

/* @fn      reset_DW IC
 * @brief   DW_RESET pin on DW IC has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW IC by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DWIC(void)
{
    nrf_gpio_cfg_output(DW3000_RESET_Pin);
    nrf_gpio_pin_clear(DW3000_RESET_Pin);
    nrf_delay_ms(2);
    nrf_gpio_cfg_input(DW3000_RESET_Pin, NRF_GPIO_PIN_NOPULL);
    nrf_delay_ms(2);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn wakeup_device_with_io()
 *
 * @brief This function wakes up the device by toggling io with a delay.
 *
 * input None
 *
 * output -None
 *
 */
void wakeup_device_with_io(void)
{
    SET_WAKEUP_PIN_IO_HIGH;
    WAIT_200uSEC;
    SET_WAKEUP_PIN_IO_LOW;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn make_very_short_wakeup_io()
 *
 * @brief This will toggle the wakeup pin for a very short time. The device should not wakeup
 *
 * input None
 *
 * output -None
 *
 */
void make_very_short_wakeup_io(void)
{
    uint8_t cnt;

    SET_WAKEUP_PIN_IO_HIGH;
    for (cnt = 0; cnt < 10; cnt++)
        __NOP();
    SET_WAKEUP_PIN_IO_LOW;
}

/****************************************************************************
 *
 *                          End APP port section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                              IRQ section
 *
 *******************************************************************************/

/* @fn      process_deca_irq
 * @brief   main call-back for processing of DW3000 IRQ
 *          it re-enters the IRQ routing and processes all events.
 *          After processing of all events, DW3000 will clear the IRQ line.
 * */
__INLINE void process_deca_irq(void)
{
    while (port_CheckEXT_IRQ() != 0)
    {
        if (port_dwic_isr)
        {
            port_dwic_isr();
        }
    } // while DW3000 IRQ line active
}

/* @fn      port_DisableEXT_IRQ
 * @brief   wrapper to disable DW_IRQ pin IRQ
 * */
__INLINE void port_DisableEXT_IRQ(void)
{
    nrf_drv_gpiote_in_event_disable(current_irq_pin);

}

/* @fn      port_EnableEXT_IRQ
 * @brief   wrapper to enable DW_IRQ pin IRQ
 * */
__INLINE void port_EnableEXT_IRQ(void)
{
    nrf_drv_gpiote_in_event_enable(current_irq_pin, true);
}

/* @fn      port_GetEXT_IRQStatus
 * @brief   wrapper to read a DW_IRQ pin IRQ status
 * */
__INLINE uint32_t port_GetEXT_IRQStatus(void)
{
    bool status = nrfx_gpiote_in_is_set(current_irq_pin);

    if (status == TRUE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* @fn      port_CheckEXT_IRQ
 * @brief   wrapper to read DW_IRQ input pin state
 * */
__INLINE uint32_t port_CheckEXT_IRQ(void)
{
    return nrf_gpio_pin_read(current_irq_pin);

}

/****************************************************************************
 *
 *                              END OF IRQ section
 *
 *******************************************************************************/

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_dwic_isr()
 *
 * @brief This function is used to install the handling function for DW IC IRQ.
 *
 * NOTE:
 *   - The user application shall ensure that a proper handler is set by calling this function before any DW IC IRQ occurs.
 *   - This function deactivates the DW IC IRQ line while the handler is installed.
 *
 * @param deca_isr function pointer to DW IC interrupt handler to install
 *
 * @return none
 */
void port_set_dwic_isr(port_dwic_isr_t dwic_isr)
{
    /* Check DW IC IRQ activation status. */
    uint8_t en = port_GetEXT_IRQStatus();

    /* If needed, deactivate DW IC IRQ during the installation of the new handler. */
    port_DisableEXT_IRQ();

    port_dwic_isr = dwic_isr;

    if (!en)
    {
        port_EnableEXT_IRQ();
    }
}

/****************************************************************************
 *
 *                              END OF Report section
 *
 *******************************************************************************/
