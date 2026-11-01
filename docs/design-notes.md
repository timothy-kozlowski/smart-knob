# Design notes: Rev 1 custom board

Decisions made on 2026-10-30, before starting the KiCad schematic.

The Adafruit Qualia ESP32-S3 RGB-666 is the reference design. The schematic is drawn from datasheets, not copied from Adafruit's files.

## Requirements

| Area | Decision |
|---|---|
| Power | USB-C 5 V. The board is ready for Li-ion: charger, power-path, protection and JST battery connector are placed as DNP footprints. The 3.3 V regulator accepts 3.0–5.5 V input. Battery sense goes on GPIO17. |
| Display | 2.1" round 480×480 RGB-666 panel (TL021WVC02-class) with capacitive touch |
| Encoder | EC11-style mechanical encoder with detents and a push switch |
| Controls | Home Assistant entities, smart lights directly (no hub), media and volume. Everything goes over the network, so the board has **no local load switching** and stays low-voltage only. |
| Connectivity | Wi-Fi (HA native API / MQTT), Matter over Wi-Fi, BLE (HID volume control, provisioning). Only the ESP32-S3's internal radio, no Thread/Zigbee chip. |
| Enclosure | Round desk puck, 3D-printed. Round PCB about 55–65 mm across, display on top, USB-C on the side. |

### Consequences
- **Regulator:** it has to run from a 1S cell (3.0–4.2 V) as well as from 5 V, so adding a battery later doesn't mean replacing it. Candidates are a buck-boost or a low-dropout LDO.
- **Antenna:** keep the module antenna's keep-out area from the WROOM-1 datasheet. Place it at the board edge, facing out of the case, away from the display FPC, any metal bezel and the ground pour.
- **Board area:** a round PCB has to fit the DNP battery parts and the antenna keep-out, so it will probably be 4 layers with parts on both sides. See *Passives and assembly*: two-sided parts are limited to ones we solder by hand.
- **Memory:** 16 MB flash and 8 MB octal PSRAM (N16R8-class module) is enough for Matter plus the display frame buffers.

## Part choices

Decided on 2026-10-31. Prices and stock are from JLCPCB on that date.

### MCU module: ESP32-S3-WROOM-1-N16R8 (LCSC C2913202)
- **Octal PSRAM (R8) is required.** One 480×480 RGB565 frame is about 450 KB, and the RGB peripheral reads it from PSRAM all the time. Quad PSRAM (R2) is too slow, and the image drifts or tears.
- **The PSRAM has to be 3.3 V. Don't use the "V" (1.8 V) parts or WROOM-2.** GPIO45 carries display B5 and has to be low at reset, which selects 3.3 V VDD_SPI.
- **16 MB flash** leaves room for two OTA slots of a Matter image (about 2 MB each) plus fonts and images.
- **16 MB PSRAM (R16V) parts aren't worth it.** They are no faster, need 1.8 V, and are harder to buy.
- **WROOM-1 instead of MINI-1, PICO-1 or a bare chip.** MINI-1 has no octal PSRAM. PICO-1 or a bare chip means designing our own RF matching and antenna, and losing the module's certification.
- **The R8 parts are rated to 65 °C ambient.** That's fine on a desk, but keep the regulator and backlight driver away from the module.
- **Fallback:** WROOM-1U-N16R8 (C3013946, external antenna on IPEX) if the enclosure ends up with a metal bezel or base.
- **KiCad:** the symbol, footprint (`RF_Module:ESP32-S3-WROOM-1`) and 3D model all come with KiCad.

### USB-C receptacle: HRO TYPE-C-31-M-12 (LCSC C165948, $0.19)
- 16-pin USB 2.0 horizontal receptacle, the usual choice on open-source ESP32 boards. Rated 5 A / 20 V.
- **Why 16-pin and not 14-pin.** The pinout of 14-pin parts differs between vendors, and some of them drop a CC pin or a data pair. KiCad has a well-tested footprint for this 16-pin part.
- **Not the GCT USB4105-GF-A** (C3020560, $1.31). It costs about 7× as much, there is far less stock, and JLCPCB may charge to solder its through-hole legs.
- **Required circuit:**
  - CC1 and CC2 each get their **own** 5.1 kΩ resistor to GND.
  - Tie both D+ pins together and both D− pins together.
  - Put ESD protection (for example USBLC6-2SC6) on D+, D− and VBUS.
- **Placement:** at the board edge, with the shell sticking out about 0.5 mm as the footprint shows, so the plug reaches through the case wall.
- **KiCad:**
  - Symbol: `Connector:USB_C_Receptacle_USB2.0_16P`
  - Footprint: `smart-knob:USB_C_Receptacle_HRO_TYPE-C-31-M-12`. This is a copy of the KiCad footprint, made because KiCad doesn't ship a 3D model for it.
  - 3D model: `${KIPRJMOD}/lib/3dmodels/TYPE-C-31-M-12.stp`, downloaded from LCSC.

### Passives and assembly
The board is assembled by hand with a stencil, so part sizes are picked for hand placement.

| Use | Size | KiCad footprint |
|---|---|---|
| All resistors and caps up to 1 µF | 0603 | `Resistor_SMD:R_0603_1608Metric`, `Capacitor_SMD:C_0603_1608Metric` |
| Bulk caps of 10 µF and up (USB, regulator, 3.3 V rail) | 0805, rated 10 V or more | `Capacitor_SMD:C_0805_2012Metric` |

- **Not 0402.** It's hard to place by hand and tends to stand up on one end during reflow. Use it only if the board runs out of room.
- **Bulk caps go in 0805** because small ceramics lose much of their capacitance under DC bias. A "22 µF" 0603 part at 5 V can act like 5–8 µF.
- **The WROOM-1 has its own decoupling.** Add 10 µF + 0.1 µF next to its 3V3 pin, and the EN pull-up and capacitor from the datasheet.
- **Stencil:** 0.12 mm thick (JLCPCB's default), framed. That thickness works for the 0603 and 0805 parts, the module and the USB-C fine-pitch pins.
- **Keep the SMD parts on the top side.** Two-sided reflow needs a second stencil, and parts on the underside can fall off during the second pass. Anything that has to go on the bottom (for example the DNP battery parts) is soldered by hand.

## GPIO allocation

Sources:
- Adafruit Qualia pinout page
- Adafruit CircuitPython `boards/adafruit_qualia_s3_rgb666/pins.c` and `mpconfigboard.h/.mk`
- arduino-esp32 `variants/adafruit_qualia_s3_rgb666/pins_arduino.h`
- ESP32-S3 and ESP32-S3-WROOM-1 datasheets

The text of the pinout page doesn't give GPIO numbers for the display, I2C or USB. Those numbers come from the CircuitPython and Arduino board definitions, which agree with each other.

### RGB display (20 GPIOs)
The ESP32-S3 LCD peripheral can drive at most 16 data lines, so the RGB-666 panel runs as RGB-565. The ESP32 doesn't drive the panel's R0 and B0.

| Signal | GPIO | Notes |
|---|---|---|
| PCLK | 1 | |
| DE | 2 | |
| HSYNC | 41 | Also JTAG MTDI. Not a problem, because debugging goes over USB-JTAG. |
| VSYNC | 42 | Also JTAG MTMS |
| R1..R5 | 11, 10, 9, 46, 3 | 46 and 3 are strapping pins |
| G0..G5 | 48, 47, 21, 14, 13, 12 | |
| B1..B5 | 40, 39, 38, 0, 45 | 0 and 45 are strapping pins. 39 and 40 are JTAG pins. |

### I2C
| Signal | GPIO |
|---|---|
| SDA | 8 |
| SCL | 18 |

The I/O expander and the touch controller share this bus.

### PCA9554A I/O expander (I2C address 0x3F, jumper-selectable 0x3B–0x3F)
| Bit | Function |
|---|---|
| 0 | TFT_SCK (panel init SPI, bit-banged) |
| 1 | TFT_CS |
| 2 | TFT_RESET |
| 3 | Touch IRQ |
| 4 | Backlight enable (on/off) |
| 5 | Button UP (Qualia only) |
| 6 | Button DOWN (Qualia only) |
| 7 | TFT_MOSI |

### USB, UART and boot
| Function | GPIO | Notes |
|---|---|---|
| USB D− / D+ | 19 / 20 | Native USB-Serial-JTAG, so no USB-UART chip |
| UART0 TX / RX | 43 / 44 | ROM boot log. Bring out to test pads. |
| BOOT | 0 | Shared with display B4 |

### Strapping pins
| GPIO | Strap | Level needed at reset | Use |
|---|---|---|---|
| 0 | Boot mode | High for normal boot, low for download mode | Display B4 and BOOT button |
| 3 | JTAG source (only if the eFuse is set) | Don't care by default | Display R5 |
| 45 | VDD_SPI voltage | **Low** for 3.3 V flash and PSRAM | Display B5 |
| 46 | Boot mode / ROM log | Low or floating in download mode | Display R4 |

These pins can carry display data because the panel inputs are high-impedance at reset. On this board, don't put pull-ups on GPIO45 or GPIO46, and make sure nothing holds GPIO0 low during reset.

### Unavailable
- 22–25 don't exist on the ESP32-S3.
- 26–32 are used by the internal SPI flash.
- 33 and 34 aren't brought out on the WROOM-1.
- 35, 36 and 37 are used by the octal PSRAM.

### Encoder and spare GPIOs
| GPIO | Use | Notes |
|---|---|---|
| 4 | ENC_A | PCNT quadrature input |
| 5 | ENC_B | PCNT quadrature input |
| 6 | ENC_SW | GPIO interrupt, debounced in a task |
| 7 | Spare | |
| 15 | Spare | |
| 16 | Backlight PWM (LEDC) | Same job as the Qualia's A1 jumper option |
| 17 | Battery voltage sense | ADC2. The divider can be DNP. |

The encoder pins match `firmware/main/board_pins.h`, so no pin change is needed.

GPIO4–6 are not strapping pins, and they are RTC GPIOs, so the knob can wake the chip from deep sleep (ext1).

Encoder circuit:
- A and B go to one PCNT unit in quadrature mode, with the glitch filter on (about 1 µs).
- A, B and SW each get a 10 k pull-up and an RC filter, with values from the encoder datasheet. The encoder common pin goes to GND.
- Check the pulses-per-detent ratio of the chosen EC11 part. Some have 1 pulse per detent and some have 2.

## Open items
- [ ] How are the panel's R0/B0 LSBs tied off? Check the Qualia schematic and the panel datasheet.
- [ ] Touch controller part number and I2C address, and the panel FPC pinout
- [ ] Regulator choice: buck-boost or LDO
- [ ] Charger, power-path and protection ICs for the DNP footprints
- [ ] Encoder placement: shaft through a centre hole, or a rotating bezel ring driving an off-centre EC11
- [ ] Gather datasheets: ESP32-S3-WROOM-1 (N16R8), PCA9554A, panel and touch controller, EC11 part
- [ ] Firmware: set `sdkconfig` to 16 MB flash and enable octal PSRAM (80 MHz), then `idf.py save-defconfig`. It is still at 2 MB flash with no PSRAM.
- [ ] Check that the Rev 0 breadboard dev board is also N16R8 (for example DevKitC-1-N16R8).
- [ ] ESD part for USB: confirm USBLC6-2SC6 or pick an alternative from JLCPCB stock.
