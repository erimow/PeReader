#define PERSIST_KEY_BRIGHTNESS 50
#ifndef SETTINGS_H
#define SETTINGS_H
#include <pebble.h>
#include <stdint.h>

void settings_page_create(GRect frame);
void settings_page_open(Window *window);
void settings_page_destroy();

// other functions
void set_brightness(bool bright);
bool is_brightness_enabled();
uint8_t get_font_size();

#endif
