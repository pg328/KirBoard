// Copyright 2024 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#ifdef OLED_ENABLE
#    include "oled_driver.h"
#    ifdef WPM_ENABLE
#        include "wpm.h"
#    endif

// Rotate per half (adjust if your screens are mounted differently)
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (!is_keyboard_left) {
        return OLED_ROTATION_180

    }
    return false
}
#endif

// -------- Layer labels --------
const char *layer_names[] = {
    "Alpha",
    "Navigation",
    "Numeric",
    "Function",
    "Mouse",
    "Shortcuts"
};

const char *layer_keymap[] = {
    "Alphabet",
    "Navigation & Media Utilities",
    "Numbers and Symbols",
    "Function Keys",
    "Mouse Control",
    "Various Shortcut Keys"
};

// -------- Kirby sprite (32x32, page-packed, 128 bytes) --------
const char kirbyM[128] = {
// Page 0 (y 0..7)
0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xC0,0xE0,0xF0,0xF0,0xF0,0xF8,0xF8,0xF8,
0xF8,0xF8,0xF8,0xF0,0xF0,0xF0,0xE0,0xC0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
// Page 1 (y 8..15)
0x00,0x00,0x00,0xE0,0xFC,0xFE,0xFF,0xFF,0xFF,0xFF,0x07,0x03,0x07,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0x07,0x03,0x07,0xFF,0xFF,0xFF,0xFE,0xFC,0xE0,0x00,0x00,0x00,
// Page 2 (y 16..23)
0x00,0x00,0x00,0x07,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFB,
0xFB,0xFB,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x87,0x00,0x00,0x00,
// Page 3 (y 24..31)
0x00,0x00,0x00,0x00,0x07,0x0F,0x1F,0x3F,0x3F,0x3F,0x3F,0x1F,0x0F,0x1F,0x1F,0x1F,
0x1F,0x1F,0x1F,0x0F,0x0F,0x1F,0x3F,0x3F,0x3F,0x3F,0x1F,0x0F,0x07,0x00,0x00,0x00
};

// Draw a 32x32 page-packed sprite at (x,y). y must be multiple of 8 or it draws wrong!!
static void draw_kirby32x32_at(uint8_t x, uint8_t y) {
    const uint8_t width = 32;
    const uint8_t pages = 4;
    const uint8_t y_page = y / 8;
    for (uint8_t p = 0; p < pages; p++) {
        uint16_t base = (y_page + p) * OLED_DISPLAY_WIDTH + x;
        for (uint8_t i = 0; i < width; i++) {
            oled_write_raw_byte((uint8_t)kirbyM[p * width + i], base + i);
        }
    }
}

#ifdef OLED_ENABLE
// -------- Event-driven OLED: only redraw when content changes or the screen stays on perpetually --------
bool oled_task_user(void) {
  if (!is_keyboard_master()) {
        return false;  // left/secondary stays blank
    }
    // get current state
    const uint8_t layer_now = get_highest_layer(layer_state | default_layer_state);
    #ifdef WPM_ENABLE
    const uint8_t wpm_now = (uint8_t)get_current_wpm();
    #else
    const uint8_t wpm_now = 0;
    #endif

    // Track last drawn state; mark dirty when something visible changes
    static uint8_t layer_prev       = 0xFF;
    static uint8_t wpm_prev_bucket  = 0xFF; // bucket WPM to avoid constant redraws
    static uint32_t last_draw       = 0;

    const uint8_t wpm_bucket = wpm_now;

    bool dirty = false;
    if (layer_now != layer_prev)        { layer_prev = layer_now; dirty = true; }
    if (wpm_bucket != wpm_prev_bucket)  { wpm_prev_bucket = wpm_bucket; dirty = true; }

    // frame cap defined by OLED_UPDATE_INTERVAL in config.h
    #ifdef OLED_UPDATE_INTERVAL
    if (dirty && timer_elapsed32(last_draw) < OLED_UPDATE_INTERVAL) return false;
    #endif
    if (!dirty) return false;                 // no writes allowed if nothing changed!!

    last_draw = timer_read32();

    // render only when dirty
    oled_clear();

    if (layer_now == 0) {
        oled_set_cursor(0, 0);
        oled_write_P(PSTR("KirBoard v1:"), false);
        oled_set_cursor(2, 4);
        oled_write_P(PSTR("Made by Phil George"), false);
        // Page-align Y (multiple of 8). 32 centers 32px kirby on 64px-tall display.
        draw_kirby32x32_at(48, 0);
    } else {
        oled_set_cursor(0, 0);
        oled_write_P(PSTR("Layer:"), false);
        if (layer_now < ARRAY_SIZE(layer_names)) {
            oled_set_cursor(7, 0);
            oled_write(layer_names[layer_now], false);
        }
        oled_set_cursor(0, 2);
        if (layer_now < ARRAY_SIZE(layer_keymap)) {
            oled_write(layer_keymap[layer_now], false);
        }
    }

    oled_set_cursor(0, 7);
    oled_write_P(PSTR("WPM:"), false);
    char wbuf[5];
    snprintf(wbuf, sizeof wbuf, "%3u", wpm_now);
    oled_write(wbuf, false);

    return false;
}
#endif

// -------- Wake OLED on any input + encoder stuff --------
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        #ifdef OLED_ENABLE
        oled_on();      // wake on any key press
        #endif
    }
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    #ifdef OLED_ENABLE
    oled_on();          // wake on encoder movement
    #endif

    if (index == 0) {
        if (layer_state_is(0)) {
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
        } else {
            tap_code16(clockwise ? C(KC_TAB) : C(S(KC_TAB)));
        }
    } else if (index == 1) {
        if (layer_state_is(0)) {
            tap_code(clockwise ? KC_RGHT : KC_LEFT);
        } else {
            tap_code(clockwise ? MS_WHLD : MS_WHLU);
        }
    }
    return false;
}

