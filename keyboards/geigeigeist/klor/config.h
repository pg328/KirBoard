// Copyright 2024 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#define MASTER_RIGHT
#define OLED_TIMEOUT 5000
#define SPLIT_ACTIVITY_ENABLE
#define OLED_UPDATE_INTERVAL 100
// #define OLED_BRIGHTNESS 128

#ifdef OLED_ENABLE
    #define OLED_DISPLAY_128X64
    #define OLED_FONT_H "keyboards/geigeigeist/klor/glcdfont.c"
#endif

// Needed for OLED display
#define I2C_DRIVER    I2CD1
#define I2C1_SDA_PIN  D0
#define I2C1_SCL_PIN  D1
