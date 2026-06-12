#define PERSIST_KEY_BRIGHTNESS 50
#define PERSIST_KEY_FONT 69
#ifndef SETTINGS_H
#define SETTINGS_H
#include <pebble.h>
#include <stdint.h>

typedef enum FontSize { TWENTYFOUR, TWENTYEIGHT } FontSize;

void settings_page_create(GRect frame, TextLayer *textlayer);
void settings_page_open(Window *window);
void settings_page_destroy();

// other functions
void set_brightness(bool bright);
bool is_brightness_enabled();
void update_font_layer_size(TextLayer *text_layer);
void set_font_size(uint8_t fontsize);
uint8_t get_font_size();

#endif
