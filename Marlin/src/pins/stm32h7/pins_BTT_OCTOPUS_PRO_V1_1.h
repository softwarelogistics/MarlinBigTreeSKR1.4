/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2023 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

#define BOARD_INFO_NAME   "BTT OCTOPUS PRO V1.1"

#include "pins_BTT_OCTOPUS_PRO_V1_common.h"

//
// Steppers
//
#define Z2_ENABLE_PIN                       PA2

//
// Heaters / Fans
//
#define HEATER_0_PIN                        PA0   // Heater0
#define HEATER_2_PIN                        PB0   // Heater2

//
// NeoPixel LED
//
#ifndef NEOPIXEL_PIN
  #define NEOPIXEL_PIN                      PB10
#endif

//
// Photon feeder RS485 (MAX485) on EXP1, only for Octopus Pro V1.1.
// UART2 TX=PD5 and RX=PD6 are exposed separately on J26.
// Keep /RE LOW so the packetizer can read back transmitted bytes;
// toggle DE HIGH for transmit and LOW to listen.
//
#if defined(RS485_SERIAL_PORT)
  #if HAS_WIRED_LCD || ENABLED(BTT_MOTOR_EXPANSION)
    #error "RS485 uses EXP1 PE7/PE8; choose new enable pins before enabling an LCD or motor expansion."
  #endif
  #define RS485_RX_ENABLE_PIN               EXP1_02_PIN // PE7 -> MAX485 /RE (active LOW)
  #define RS485_TX_ENABLE_PIN               EXP1_01_PIN // PE8 -> MAX485 DE  (active HIGH)
#endif
