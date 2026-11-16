# PNP Vacuum Raw Count Bridge

The ESP32 reads two XGZP6857D sensors and streams signed raw 24-bit counts via 3.3 V UART2 to the Octopus Pro USART3. Marlin caches the last valid frame and exposes it to the host when M170 is requested. The shared C# manufacturing service owns calibration, averaging, and part detection.

## Wiring and baud

- PRS1 sensor: SDA GPIO21 / SCL GPIO22 (I2C bus 0)
- PRS2 sensor: SDA GPIO26 / SCL GPIO23 (I2C bus 1)
- Both independent sensors use I2C address 0x6D (no multiplexer on the ESP32 board)
- ESP32 TX2 GPIO17 -> Octopus RX3
- ESP32 RX2 GPIO16 <- Octopus TX3 (currently unused)
- ESP32 ground <-> Octopus ground; 3.3 V TTL serial only
- UART at 115200 baud, 8N1; verify GPIO16/17 wiring before connecting

## Protocol

ESP32 streams a complete frame every ~50 ms on UART2 and mirrors the same frame over USB:

    prs1:-123456 prs2:7890

These are signed raw sensor counts, not calibrated pressure or 0-10000 values. Marlin UART3 never interprets ESP32 input as G-code. Host sends M170; Marlin replies:

    prs1:-123456 prs2:7890
    ok

If no valid frame arrived within 750 ms, Marlin emits error:vacuum-stale followed by its normal acknowledgement. No sensor failure is silently turned into a zero reading.

The old manufacturing parser assigns prs1 to the right head and prs2 to the left; confirm your actual physical mapping when integrating C#.

## Build

- ESP32: open PNPVacuumFirmware as the root PlatformIO project (esp32dev / Arduino).
- Marlin: on serial-only branch at repo root, run pio run -e STM32H723ZE_btt and flash via the existing Octopus process.
- Native framing tests: g++ -std=c++11 -Wall -Wextra -Werror -I Marlin/src/gcode test/pnp_vacuum_protocol_native.cpp -o /tmp/pnp-test && /tmp/pnp-test

Only native frame-parser tests have been executed here. Marlin and ESP32 full PlatformIO builds and physical UART validation still need to be performed.
