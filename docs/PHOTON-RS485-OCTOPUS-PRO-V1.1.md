# Opulo Photon feeders over RS485 - BTT Octopus Pro V1.1

**Firmware branch:** `serial-only`  
**Target:** `STM32H723ZE_btt` / `BOARD_BTT_OCTOPUS_PRO_V1_1`

This enables the **existing** Marlin `M485` / Photon packetizer; it does not implement feeder commands in Marlin. The existing C# `PhotonProtocolHandler` in `LagoVista/manufacturing` builds packets and parses `rs485-reply: ...` responses.

## Dedicated pins

| Octopus Pro V1.1 | Header | MAX485ESA breakout | Purpose |
| --- | --- | --- | --- |
| PD5 / USART2_TX | UART2 J26 | DI | MCU transmit |
| PD6 / USART2_RX | UART2 J26 | RO | MCU receive |
| PE7 / EXP1 pin 2 | EXP1 | RE (active LOW) | Receiver held enabled |
| PE8 / EXP1 pin 1 | EXP1 | DE (active HIGH) | Transmitter direction |
| 5V | UART2 J26 | VCC | MAX485ESA is a **5V** transceiver |
| GND | UART2 J26 | GND | Shared logic ground |
| -- | -- | A / B | To feeder **RS485 A/B only** (verify feeder pinout) |

**Do not join DE and RE.** The installed `jnesselr/rs485` library drives the read-enable pin LOW and uses local receive echo to verify sent bytes and check collisions. Its write-enable pin transitions LOW/HIGH/LOW. Two separate GPIOs are assigned accordingly.

**Electrical checks before connecting:** The MAX485's RO output may swing to 5V. Confirm that PD6 is correctly configured as a 5V-tolerant digital input for this STM32H723 board, or use a proper 5V-to-3.3V level shifter on RO. Do **not** feed 5V into an unverified MCU input. Add a pull-down on DE if necessary to keep the bus inactive during reset. The feeder requires its own appropriate power supply (typically 24V); **never feed 24V into the Octopus or MAX485 logic pins**. Verify the exact feeder connector wiring and differential A/B polarity before applying power, and share an appropriate ground reference.

EXP1 pins are dedicated to RS485 in this configuration; a wired LCD or BTT motor expansion must not use the same pins. The pin file has a build-time guard for these two cases. Check that no external hardware is already attached to EXP1.

## Marlin configuration

- `RS485_SERIAL_PORT 2`: UART2 dedicated to feeders, initialized by `rs485_init()` at **57600 baud**.
- `M485_PROTOCOL 1`: emits `rs485-reply: <HEX>` / `rs485-reply: TIMEOUT`, matching the existing C# parser.
- `RS485_BUS_BUFFER_SIZE 128`.
- Existing `SERIAL_PORT 1`, `SERIAL_PORT_2 -1` (USB), and `SERIAL_PORT_3 3` (ESP32 vacuum bridge) remain unchanged.

## First bench test (after electrical verification)

1. Build `pio run -e STM32H723ZE_btt`. Do not flash a build that fails.
2. Wire the breakout with feeder **power disconnected**; verify 5V and GND first. Confirm the RX logic voltage before attaching RO.
3. Boot Marlin and confirm `M115` still works. Run `M170` with the ESP32 connected to confirm the vacuum bridge is unaffected.
4. With the transceiver connected but no feeder, send `M485 010000017001`. This is the C# `GetId` packet for slot 1, packet ID 0, CRC 0x70. A timeout is expected if no device is connected; it confirms the command path is available, **not** that the RS485 electrical link works.
5. After verifying the exact Opulo feeder wiring, power one feeder and send its GetId command. Expect `rs485-reply: <hex>` with a valid CRC and matching packet ID; then test a single small advance.
6. Only then try multi-feeder discovery and operation.

Note: `M485` is a synchronous G-code command; do not run a multi-feeder scan before single-feeder responses are stable.

## References

- [Octopus Pro V1.1 schematic](https://github.com/bigtreetech/BIGTREETECH-OCTOPUS-Pro/blob/master/Hardware/BIGTREETECH%20Octopus%20Pro%20V1.1-sch.pdf)
- [Marlin RS485 source](https://github.com/softwarelogistics/MarlinBigTreeSKR1.4/blob/serial-only/Marlin/src/gcode/feature/rs485/M485.cpp)
- [C# PhotonProtocolHandler](https://github.com/LagoVista/manufacturing/blob/master/src/LagoVista.PickAndPlace/LumenSupport/PhotonProtocolHandler.cs)
- [RS485 library receive-echo implementation](https://github.com/Jnesselr/RS485/blob/main/src/rs485/rs485bus_base.cpp)
