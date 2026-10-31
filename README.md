# Smart Knob
Rotary knob + round display smart home controller (ESP32-S3).

## Structure
- `hardware/` – KiCad project (open `smart-knob.kicad_pro`)
- `firmware/` – ESP-IDF project
- `docs/` – datasheets and notes

## Build firmware
Requires ESP-IF v6.1
cd firmware
idf.py build
idf.py -p <PORT> flash monitor

## Hardware status
Rev 0: breadboard prototype