#include "settings.h"
#include <stdint.h>

// static int8_t font;
static MenuLayer *menu_layer;

// other
static bool brightness;
static FontSize font_size = TWENTYFOUR;
static TextLayer *p_to_main_text_layer;

// CALLBACKS
//

uint16_t menu_get_num_rows(MenuLayer *menulayer, uint16_t section_index,
                           void *context) {
  return 3;
}

void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer,
                            MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
  case 0:
    menu_cell_basic_draw(ctx, cell_layer, "Backlight",
                         (brightness) ? "on" : "off", NULL);
    break;
  case 1:
    menu_cell_basic_draw(ctx, cell_layer, "Font Size",
                         (font_size == TWENTYFOUR) ? "24" : "28", NULL);
    break;
  case 2:
    menu_cell_basic_draw(ctx, cell_layer, "Exit Settings", "",
                         NULL); // look into other draw functions for this one
    break;
  }
}

void menu_select_callback(MenuLayer *menulayer, MenuIndex *cell_index,
                          void *ctx) {
  switch (cell_index->row) {
  case 0: // Backlight
    // BACKLIGHT ON
    brightness = !brightness;
    persist_write_bool(PERSIST_KEY_BRIGHTNESS, brightness);
    // (brightness)
    //     ? action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP,
    //                                          brightness_on_image, true)
    //     : action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP,
    //                                          brightness_off_image, true);
    light_enable(brightness);
    layer_mark_dirty(menu_layer_get_layer(menu_layer));
    break;
  case 1: // Font Size
    switch (font_size) {
    case TWENTYFOUR:
      font_size = TWENTYEIGHT;
      break;
    case TWENTYEIGHT:
      font_size = TWENTYFOUR;
      break;
    }
    update_font_layer_size(p_to_main_text_layer);
    persist_write_int(PERSIST_KEY_FONT, font_size);
    layer_mark_dirty(menu_layer_get_layer(menu_layer));
    break;
  case 2:
    window_stack_pop(true);
    break;
  }
}

void settings_page_create(GRect frame, TextLayer *textlayer) {
  p_to_main_text_layer = textlayer;
  menu_layer = menu_layer_create(frame);
  menu_layer_set_callbacks(
      menu_layer, NULL,
      (MenuLayerCallbacks){.get_num_rows = menu_get_num_rows,
                           .draw_row = menu_draw_row_callback,
                           .select_click = menu_select_callback});
}
void settings_page_open(Window *window) {
  menu_layer_set_click_config_onto_window(menu_layer, window);
  layer_add_child(window_get_root_layer(window),
                  menu_layer_get_layer(menu_layer));
  layer_mark_dirty(menu_layer_get_layer(menu_layer));
}
void settings_page_destroy() {
  if (menu_layer)
    menu_layer_destroy(menu_layer);
}

void set_brightness(bool bright) { brightness = bright; }

bool is_brightness_enabled() { return brightness; }

void set_font_size(uint8_t fontsize) { font_size = fontsize; }

void update_font_layer_size(TextLayer *text_layer) {
  if (get_font_size() == TWENTYFOUR) {
    text_layer_set_font(text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  } else if (get_font_size() == TWENTYEIGHT) {

    text_layer_set_font(text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  }
}

uint8_t get_font_size() { return font_size; }
