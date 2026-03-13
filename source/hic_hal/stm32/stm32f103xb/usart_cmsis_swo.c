/**
 * @file    usart_cmsis_swo.c
 * @brief   CMSIS USART driver instance for SWO capture on STM32F103XB.
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2021, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stm32f1xx.h"
#include "Driver_USART.h"

#define SWO_USART                     USART1
#define SWO_USART_CLK_ENABLE()        __HAL_RCC_USART1_CLK_ENABLE()
#define SWO_USART_CLK_DISABLE()       __HAL_RCC_USART1_CLK_DISABLE()
#define SWO_USART_IRQn                USART1_IRQn
#define SWO_USART_IRQ_HANDLER         USART1_IRQHandler

#define SWO_UART_PINS_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define SWO_UART_RX_PORT              GPIOA
#define SWO_UART_RX_PIN               GPIO_PIN_10

static ARM_USART_SignalEvent_t usart_cb;
static ARM_USART_STATUS usart_status;

static uint8_t *rx_buf;
static uint32_t rx_num;
static volatile uint32_t rx_cnt;

static uint8_t usart_initialized;
static uint8_t usart_powered;
static uint8_t usart_configured;
static uint8_t rx_enabled;

static ARM_DRIVER_VERSION ARM_USART0_GetVersion(void)
{
    ARM_DRIVER_VERSION version = { ARM_USART_API_VERSION, 0x0100U };
    return version;
}

static ARM_USART_CAPABILITIES ARM_USART0_GetCapabilities(void)
{
    ARM_USART_CAPABILITIES caps;

    caps.asynchronous = 1U;
    caps.synchronous_master = 0U;
    caps.synchronous_slave = 0U;
    caps.single_wire = 0U;
    caps.irda = 0U;
    caps.smart_card = 0U;
    caps.smart_card_clock = 0U;
    caps.flow_control_rts = 0U;
    caps.flow_control_cts = 0U;
    caps.event_tx_complete = 0U;
    caps.event_rx_timeout = 0U;
    caps.rts = 0U;
    caps.cts = 0U;
    caps.dtr = 0U;
    caps.dsr = 0U;
    caps.dcd = 0U;
    caps.ri = 0U;
    caps.event_cts = 0U;
    caps.event_dsr = 0U;
    caps.event_dcd = 0U;
    caps.event_ri = 0U;
    caps.reserved = 0U;

    return caps;
}

static int32_t ARM_USART0_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    usart_cb = cb_event;
    usart_status.tx_busy = 0U;
    usart_status.rx_busy = 0U;
    usart_status.tx_underflow = 0U;
    usart_status.rx_overflow = 0U;
    usart_status.rx_break = 0U;
    usart_status.rx_framing_error = 0U;
    usart_status.rx_parity_error = 0U;
    usart_initialized = 1U;
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART0_Uninitialize(void)
{
    usart_initialized = 0U;
    usart_cb = NULL;
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART0_PowerControl(ARM_POWER_STATE state)
{
    GPIO_InitTypeDef gpio_init;

    if (!usart_initialized) {
        return ARM_DRIVER_ERROR;
    }

    switch (state) {
        case ARM_POWER_OFF:
            NVIC_DisableIRQ(SWO_USART_IRQn);
            SWO_USART->CR1 = 0U;
            SWO_USART->CR2 = 0U;
            SWO_USART->CR3 = 0U;
            SWO_USART_CLK_DISABLE();
            usart_powered = 0U;
            usart_configured = 0U;
            rx_enabled = 0U;
            rx_buf = NULL;
            rx_num = 0U;
            rx_cnt = 0U;
            usart_status.rx_busy = 0U;
            return ARM_DRIVER_OK;

        case ARM_POWER_FULL:
            SWO_UART_PINS_CLK_ENABLE();
            SWO_USART_CLK_ENABLE();

            gpio_init.Pin = SWO_UART_RX_PIN;
            gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
            gpio_init.Mode = GPIO_MODE_INPUT;
            gpio_init.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(SWO_UART_RX_PORT, &gpio_init);

            NVIC_ClearPendingIRQ(SWO_USART_IRQn);
            NVIC_EnableIRQ(SWO_USART_IRQn);

            usart_powered = 1U;
            return ARM_DRIVER_OK;

        case ARM_POWER_LOW:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static int32_t ARM_USART0_Send(const void *data, uint32_t num)
{
    (void)data;
    (void)num;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static int32_t ARM_USART0_Receive(void *data, uint32_t num)
{
    if (!usart_powered || !usart_configured) {
        return ARM_DRIVER_ERROR;
    }
    if ((data == NULL) || (num == 0U)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (usart_status.rx_busy) {
        return ARM_DRIVER_ERROR_BUSY;
    }

    rx_buf = (uint8_t *)data;
    rx_num = num;
    rx_cnt = 0U;

    usart_status.rx_busy = 1U;
    usart_status.rx_overflow = 0U;
    usart_status.rx_break = 0U;
    usart_status.rx_framing_error = 0U;
    usart_status.rx_parity_error = 0U;

    SWO_USART->CR1 |= USART_CR1_RXNEIE | USART_CR1_PEIE;
    SWO_USART->CR3 |= USART_CR3_EIE;

    return ARM_DRIVER_OK;
}

static int32_t ARM_USART0_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    (void)data_out;
    (void)data_in;
    (void)num;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART0_GetTxCount(void)
{
    return 0U;
}

static uint32_t ARM_USART0_GetRxCount(void)
{
    return rx_cnt;
}

static int32_t ARM_USART0_Control(uint32_t control, uint32_t arg)
{
    uint32_t mode = control & ARM_USART_CONTROL_Msk;

    if (!usart_powered) {
        return ARM_DRIVER_ERROR;
    }

    switch (mode) {
        case ARM_USART_MODE_ASYNCHRONOUS:
            if ((control & ARM_USART_DATA_BITS_Msk) != ARM_USART_DATA_BITS_8) {
                return ARM_USART_ERROR_DATA_BITS;
            }
            if ((control & ARM_USART_PARITY_Msk) != ARM_USART_PARITY_NONE) {
                return ARM_USART_ERROR_PARITY;
            }
            if ((control & ARM_USART_STOP_BITS_Msk) != ARM_USART_STOP_BITS_1) {
                return ARM_USART_ERROR_STOP_BITS;
            }
            if ((control & ARM_USART_FLOW_CONTROL_Msk) != ARM_USART_FLOW_CONTROL_NONE) {
                return ARM_USART_ERROR_FLOW_CONTROL;
            }
            if (arg == 0U) {
                return ARM_USART_ERROR_BAUDRATE;
            }

            SWO_USART->CR1 = 0U;
            SWO_USART->CR2 = 0U;
            SWO_USART->CR3 = 0U;
            SWO_USART->BRR = (HAL_RCC_GetPCLK2Freq() + (arg / 2U)) / arg;
            SWO_USART->CR1 = USART_CR1_UE;
            usart_configured = 1U;
            return ARM_DRIVER_OK;

        case ARM_USART_CONTROL_RX:
            if (!usart_configured) {
                return ARM_DRIVER_ERROR;
            }
            if (arg != 0U) {
                SWO_USART->CR1 |= USART_CR1_RE;
                rx_enabled = 1U;
            } else {
                SWO_USART->CR1 &= ~(USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_PEIE);
                SWO_USART->CR3 &= ~USART_CR3_EIE;
                rx_enabled = 0U;
            }
            return ARM_DRIVER_OK;

        case ARM_USART_ABORT_RECEIVE:
            SWO_USART->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_PEIE);
            SWO_USART->CR3 &= ~USART_CR3_EIE;
            usart_status.rx_busy = 0U;
            return ARM_DRIVER_OK;

        case ARM_USART_CONTROL_TX:
        case ARM_USART_ABORT_SEND:
            return ARM_DRIVER_OK;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static ARM_USART_STATUS ARM_USART0_GetStatus(void)
{
    return usart_status;
}

static int32_t ARM_USART0_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    (void)control;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART0_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem = {0U};
    return modem;
}

void SWO_USART_IRQ_HANDLER(void)
{
    uint32_t sr = SWO_USART->SR;
    uint32_t event = 0U;

    if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
        if (sr & USART_SR_ORE) {
            usart_status.rx_overflow = 1U;
            event |= ARM_USART_EVENT_RX_OVERFLOW;
        }
        if (sr & USART_SR_FE) {
            usart_status.rx_framing_error = 1U;
            event |= ARM_USART_EVENT_RX_FRAMING_ERROR;
        }
        if (sr & USART_SR_PE) {
            usart_status.rx_parity_error = 1U;
            event |= ARM_USART_EVENT_RX_PARITY_ERROR;
        }
    }

    if (sr & USART_SR_RXNE) {
        uint8_t data = (uint8_t)SWO_USART->DR;

        if (usart_status.rx_busy && rx_enabled && (rx_buf != NULL) && (rx_cnt < rx_num)) {
            rx_buf[rx_cnt++] = data;
            if (rx_cnt >= rx_num) {
                usart_status.rx_busy = 0U;
                SWO_USART->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_PEIE);
                SWO_USART->CR3 &= ~USART_CR3_EIE;
                event |= ARM_USART_EVENT_RECEIVE_COMPLETE;
            }
        }
    } else if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
        (void)SWO_USART->DR;
    }

    if ((event != 0U) && (usart_cb != NULL)) {
        usart_cb(event);
    }
}

ARM_DRIVER_USART Driver_USART0 = {
    ARM_USART0_GetVersion,
    ARM_USART0_GetCapabilities,
    ARM_USART0_Initialize,
    ARM_USART0_Uninitialize,
    ARM_USART0_PowerControl,
    ARM_USART0_Send,
    ARM_USART0_Receive,
    ARM_USART0_Transfer,
    ARM_USART0_GetTxCount,
    ARM_USART0_GetRxCount,
    ARM_USART0_Control,
    ARM_USART0_GetStatus,
    ARM_USART0_SetModemControl,
    ARM_USART0_GetModemStatus
};
