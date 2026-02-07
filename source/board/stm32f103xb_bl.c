/**
 * @file    stm32f103_bl.c
 * @brief   board ID and meta-data for the hardware interface circuit (HIC) based on STM32F103XB
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2019, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "target_config.h"
#include "target_board.h"
#include "target_family.h"
#include "stm32f1xx.h"
#include <stdbool.h>

// NF-RST Button on PB15 for entering maintenance mode
#define NF_RST_BTN_PORT  GPIOB
#define NF_RST_BTN_PIN   GPIO_PIN_15

// Initialize NF-RST button pin as input with pull-up for bootloader
void board_bootloader_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Configure PB15 (NF-RST button) as input with pull-up
    GPIO_InitStructure.Pin = NF_RST_BTN_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(NF_RST_BTN_PORT, &GPIO_InitStructure);
}

// Override reset_button_pressed to check PB15 (NF-RST button).
// This prevents the bootloader from staying in maintenance mode
// when no target is connected (which can cause the reset pin to float low).
// Hold NF-RST button while plugging in USB to enter maintenance mode.
bool reset_button_pressed(void)
{
    // Button is active low (pressed = 0)
    return (NF_RST_BTN_PORT->IDR & NF_RST_BTN_PIN) == 0;
}

/**
* List of start and size for each size of flash sector
* The size will apply to all sectors between the listed address and the next address
* in the list.
* The last pair in the list will have sectors starting at that address and ending
* at address start + size.
*/
static const sector_info_t sectors_info[] = {
    {0x08000000 + KB(48), 0x400},
 };

// stm32f103 target information
target_cfg_t target_device = {
    .version                    = kTargetConfigVersion,
    .sectors_info               = sectors_info,
    .sector_info_length         = (sizeof(sectors_info))/(sizeof(sector_info_t)),
    .flash_regions[0].start     = 0x08000000 + KB(48),
    .flash_regions[0].end       = 0x08000000 + KB(128),
    .flash_regions[0].flags     = kRegionIsDefault,
    .ram_regions[0].start       = 0x20000000,
    .ram_regions[0].end         = 0x20005000,
    /* .flash_algo not needed for bootloader */
};

//bootloader has no family
const target_family_descriptor_t *g_target_family = NULL;

const board_info_t g_board_info = {
    .info_version = kBoardInfoVersion,
    .board_id = "0000",
    .daplink_url_name =       "HELP_FAQHTM",
    .daplink_drive_name = 		"MAINTENANCE",
    .daplink_target_url = "https://daplink.io",
    .target_cfg = &target_device,
};
