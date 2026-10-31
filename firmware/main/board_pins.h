#pragma once
#include "driver/gpio.h"

// Names match net labels in hardware/smart-knob.kicad_sch
// Rev 0: breadboard prototype

#define PIN_ENC_A    GPIO_NUM_4   // ← replace with your real pins when prototyping
#define PIN_ENC_B    GPIO_NUM_5
#define PIN_ENC_SW   GPIO_NUM_6