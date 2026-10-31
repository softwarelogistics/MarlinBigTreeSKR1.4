Project:       D:\firmware\MarlinBigTreeSKR1.4
Branch:        serial-only
Environment:   STM32H723ZE_btt
Board:         BIGTREETECH Octopus Pro V1.1
MCU:           STM32H723ZET6
Flash address: 0x08020000
Serial port:   COM5
Firmware:      Marlin sl-2.1.x


 pio run -e STM32H723ZE_btt
 pio run -e STM32H723ZE_btt -t upload



 Yes — that's the correct microSD deployment procedure! With one terminology correction: you're flashing the firmware into the STM32's internal flash memory, not EEPROM.

For your Octopus Pro V1.1:
1. Copy .pio\build\STM32H723ZE_btt\firmware.bin to the root of a FAT32-formatted microSD card.
2. Power down the board and disconnect USB.
3. Remove the BOOT0 jumper so the board boots normally.
4. Insert the microSD card.
5. Power up the board.
6. Check whether firmware.bin gets renamed to FIRMWARE.CUR — that's the usual indication that the SD bootloader processed it.
7. Connect to COM5 and send M115 to verify.
One important caveat: We confirmed that Marlin runs after DFU flashing, but haven't yet confirmed the microSD bootloader survived our earlier upload at 0x08000000.

If the file isn't renamed, don't assume the firmware is bad. We may need to restore the H723 microSD bootloader.

Also, since you already flashed the correct H723 firmware via DFU, it should survive power cycling right now. The microSD procedure is an alternative way to install future firmware versions, not an additional step required to save the current one.

https://chatgpt.com/c/6ae5f5c6-79a0-83e8-8d71-e891822a04d2

For your Octopus Pro V1.1, the workflow is:
1. Make your code changes.
2. Run Build in PlatformIO.
3. Install the BOOT0 jumper and reset the board to enter DFU mode.
4. Run Upload in PlatformIO (STM32H723ZE_btt).
5. Remove the BOOT0 jumper.
6. Reset the board again to run your new firmware.
Important: You don't need to move the jumper for every compile — only when you're ready to upload new firmware.

And since the DFU upload writes persistent flash, your new firmware will survive power cycles.

Once we get your ST-Link connected, we can make the development/debugging cycle even more convenient!