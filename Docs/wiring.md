# Wiring Guide

This project uses an ESP32 with a CC1101 868 MHz RF module.

## Important

The CC1101 is a **3.3V device**.

Do **not** connect the CC1101 VCC pin to 5V.

Using 5V may permanently damage the CC1101 module.

## ESP32 to CC1101 wiring

| CC1101 pin | ESP32 pin | Notes |
|---|---:|---|
| VCC | 3.3V | Do not use 5V |
| GND | GND | Common ground |
| SCK / SCLK | GPIO18 | SPI clock |
| MOSI / SI | GPIO23 | ESP32 to CC1101 |
| MISO / SO / GDO1 | GPIO19 | CC1101 to ESP32 |
| CSN / CS / SS | GPIO5 | SPI chip select |
| GDO0 | GPIO4 | Optional, used by sniffer/packet detection |

## Pin aliases

Different CC1101 modules use different labels.

| Label on module | Meaning |
|---|---|
| SCK / SCLK | SPI clock |
| SI / MOSI | SPI data from ESP32 to CC1101 |
| SO / MISO / GDO1 | SPI data from CC1101 to ESP32 |
| CSN / CS / SS | SPI chip select |
| GDO0 | CC1101 digital output pin |

## Antenna

Use a CC1101 module designed for **868 MHz**.

A quarter-wave wire antenna for 868 MHz is approximately:

```text
8.2 cm to 8.6 cm