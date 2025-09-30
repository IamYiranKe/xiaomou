# Xiaomou LCD Module

## General Specifications
- **LCD Type:** 3.5" TFT LCD (transmissive)
- **Display Color:** 262K
- **Driver IC:** ST7796S
- **Duty:** 1/480
- **Bias:** Not specified
- **Viewing Direction:** All directions (0°–360°)
- **Active Area (W × H):** 48.96 mm × 73.44 mm
- **Resolution:** 320 (RGB) × 480 dots
- **Dot Pitch (W × H):** 0.153 mm × 0.153 mm
- **Supply Voltage (VDD):** 2.7 V – 3.3 V
- **Backlight:** White LED
- **Luminance:** 250 cd/m²
- **Operating Temperature:** –20 °C to +70 °C
- **Storage Temperature:** –30 °C to +80 °C
- **Data Interface:** 4-wire SPI
- **Outline Dimensions:** Refer to manufacturer drawing (see original datasheet)

## 20‑Pin FPC Interface
| Pin | Symbol | Description                        |
|-----|--------|------------------------------------|
| 1   | CTP_RST | Capacitive touch reset            |
| 2   | CTP_INT | Capacitive touch interrupt        |
| 3   | CTP_SDA | Capacitive touch I²C data         |
| 4   | CTP_SCL | Capacitive touch I²C clock        |
| 5   | CTP_GND | Capacitive touch ground           |
| 6   | CTP_VDD | Capacitive touch power (3.3 V)    |
| 7   | CTP_GND | Capacitive touch ground           |
| 8   | LED_A   | Backlight anode (positive)        |
| 9   | LED_K   | Backlight cathode (negative)      |
| 10  | GND     | Ground                            |
| 11  | SDO     | LCD data out / MISO               |
| 12  | SDI     | LCD data in / MOSI                |
| 13  | NC      | Not connected                     |
| 14  | SCK     | LCD serial clock                  |
| 15  | RS      | Register select (D/CX)            |
| 16  | CS      | Chip select                        |
| 17  | VCI     | Logic supply (2.7 V – 3.3 V)      |
| 18  | IOVCC   | I/O supply (2.7 V – 3.3 V)        |
| 19  | GND     | Ground                            |
| 20  | GND     | Ground                            |

This summary transcribes the information from `debug/lcd/spec.png`, `debug/lcd/pinout.png`, and the initialization script `debug/lcd/3.5-36A(1)(1).txt` for quick reference in version control.
