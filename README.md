# LVGL V8.3 ported to the GD32C231C-EVAL

This project ports `LVGL V8.3` to the `GD32C231C-EVAL` platform for GUI demonstrations.

## Hardware Information

The `GD32C231C-EVAL` is based on:

- `GD32C231C8T6` microcontroller (`ARM Cortex-M23` core, maximum frequency `48 MHz`)
- `64 KB` on-chip Flash memory and `12 KB` on-chip SRAM
- `2.2-inch` `<TFT>` display (`<240*320>`, `<SPI>`)
- Keys: `WakeUp=PA0` (active high), `UserKey=PA4` (active low)
- `On-board GD-LINK debugger/programmer for one-click download and debugging`
- `I2C / I2S / ADC / LED / KEY / SPI`

## Project Information

- GUI framework: `LVGL V8.3.11`
- Toolchain: `Keil MDK / IAR / GD32 Embedded Builder`
- Target board: `<GD32C231C-EVAL V1.1>`
- Display configuration: `240 x 320 / RGB565 (16-bit) / portrait orientation`


## Third-Party Components

| Category | In use | Component | Version    | License |
| -------- | ------ | --------- | ---------- | ------- |
| GUI      | `Yes`  | `LVGL`    | `V 8.3.11` | `MIT`   |

> When introducing a new third-party library, update this table and retain its license text and copyright notices.
