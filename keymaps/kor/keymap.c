/* Copyright 2020 Jay Greco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include QMK_KEYBOARD_H
#include "action_layer.h"
#include "remote_kb.h"
#include "bitc_led.h"

#define _BASE     0
#define _FUNC     1

extern rgblight_config_t rgblight_config;

enum custom_keycodes {
  PROG = SAFE_RANGE,
};

enum td_keycodes {
    TD_ENTER_LAYER
};

// Tap Dance definitions
qk_tap_dance_action_t tap_dance_actions[] = {
    // Tap once for KP_ENTER, twice for _FUNC layer
    [TD_ENTER_LAYER] = ACTION_TAP_DANCE_LAYER_TOGGLE(KC_KP_ENTER, 1),
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT(
           KC_ASTR, KC_SLSH, TD(TD_ENTER_LAYER), \
  RGB_TOG, KC_9, KC_6, KC_3, \
  KC_HOME, KC_8, KC_5, KC_2, \
  KC_MPLY, KC_7, KC_4, KC_1, \
  KC_MUTE, KC_PLUS, KC_MINS, KC_0 \
  ),
  // Function layer (numpad)
  [_FUNC] = LAYOUT(
           KC_NO, RGB_TOG, KC_NO,
    KC_NO, KC_NO, RGB_MOD, KC_NO,
    KC_NO, KC_NO, RGB_HUI, KC_NO,
    KC_NO, KC_NO, RGB_SAI, KC_NO,
    PROG,  KC_NO, RGB_VAI, TO(_BASE)
  ),
};

#ifdef OLED_DRIVER_ENABLE
// void render_rgb_status(void) {
//     oled_write_ln("RGB:", false);
//     static char temp[20] = {0};
//     snprintf(temp, sizeof(temp) + 1, "M:%3dH:%3dS:%3dV:%3d", rgb_matrix_config.mode, rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v);
//     oled_write(temp, false);
// }

void oled_task_user(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("layer: "), false);

    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR("norm\n"), false);
            break;
        case _FUNC:
            oled_write_P(PSTR("func\n"), false);
            break;
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            oled_write_ln_P(PSTR("undefined"), false);
    }

    #ifdef RGBLIGHT_ENABLE
    static char rgbStatusLine1[26] = {0};
    snprintf(rgbStatusLine1, sizeof(rgbStatusLine1), "rgb mode: %d", rgblight_get_mode());
    oled_write_ln(rgbStatusLine1, false);
    static char rgbStatusLine2[26] = {0};
    snprintf(rgbStatusLine2, sizeof(rgbStatusLine2), "h:%d s:%d v:%d", rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val());
    oled_write_ln(rgbStatusLine2, false);
    #endif

    char wpm_counter[4];
    snprintf(wpm_counter, sizeof(wpm_counter), "%3u", get_current_wpm());
    oled_write_P(PSTR(OLED_RENDER_WPM_COUNTER), false);
    oled_write(wpm_counter, false);

    // // wpm
    // char wpm_str[12];
    // sprintf(wpm_str, "[WPM] %03d \n", get_current_wpm());
    // oled_write_P(wpm_str, false);
    // oled_write_ln_P(rgblight_get_mode(), true);
    // oled_write_ln_P(PSTR("helloooo there"), true);
    // oled_write_ln_P(PSTR("helooooo there"), true);

    // render_rgb_status();

    // // Host Keyboard LED Status
    // led_t led_state = host_keyboard_led_state();
    // oled_write_P(led_state.num_lock ? PSTR("NUM ") : PSTR("    "), false);
    // oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
    // oled_write_P(led_state.scroll_lock ? PSTR("SCR ") : PSTR("    "), false);
}
#endif

void matrix_init_user(void) {
  matrix_init_remote_kb();
  register_code(KC_NLCK);
}

void matrix_scan_user(void) {
  matrix_scan_remote_kb();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  process_record_remote_kb(keycode, record);

  switch(keycode) {
    case PROG:
      if (record->event.pressed) {
        set_bitc_LED(LED_DIM);
        rgblight_disable_noeeprom();
        bootloader_jump(); //jump to bootloader
      }
    break;

    default:
    break;
  }
  return true;
}

void encoder_update_user(uint8_t index, bool clockwise) {
  if (clockwise) {
    tap_code(KC_VOLU);
  } else {
    tap_code(KC_VOLD);
  }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case _FUNC:
        unregister_code(KC_NLCK);
        break;
    default: //  for any other layers, or the default layer
        register_code(KC_NLCK);
        break;
    }
  return state;
}

void led_set_kb(uint8_t usb_led) {
  if (usb_led & (1<<USB_LED_NUM_LOCK))
    set_bitc_LED(LED_DIM);
  else
    set_bitc_LED(LED_OFF);
}
