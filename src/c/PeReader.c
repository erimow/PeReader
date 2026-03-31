#include <pebble.h>
#include <stdbool.h>
#include "message_keys.auto.h"

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_bar_layer;

//BMPS
static GBitmap *done_image;
static GBitmap *brightness_on_image;
static GBitmap *brightness_off_image;
static GBitmap *skip_image;

static int current_page = 0;
static int total_pages = 0;

static bool brightness = false;

#define PERSIST_KEY_PAGE 1
#define PERSIST_KEY_BRIGHTNESS 50

static void click_config_provider(void *context);

static void update_display(const char *text) {
  static char buffer[512];

  snprintf(buffer, sizeof(buffer), "Page %d/%d\n%s",
           current_page + 1, total_pages, text);

  text_layer_set_text(text_layer, buffer);
}

static void request_page(int page) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) return;

  dict_write_int(iter, MESSAGE_KEY_PAGE_REQUEST, &page, sizeof(int), true);
  app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *text_tuple = dict_find(iter, MESSAGE_KEY_PAGE_TEXT);
  Tuple *total_tuple = dict_find(iter, MESSAGE_KEY_TOTAL_PAGES);

  if (total_tuple) {
    total_pages = total_tuple->value->int32;
  }

  if (text_tuple) {
    update_display(text_tuple->value->cstring);
  }
}

static void action_bar_down_click_handler(ClickRecognizerRef recognizer, void *context){
  //SKIP SECTIONS OPTION
}
static void action_bar_up_click_handler(ClickRecognizerRef recognizer, void *context){
  //BACKLIGHT ON
  brightness = !brightness;
  persist_write_bool(PERSIST_KEY_BRIGHTNESS, brightness);
  (brightness) ? action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_on_image, true) : action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_off_image, true);
  light_enable(brightness);
}
static void action_bar_select_click_handler(ClickRecognizerRef recognizer, void *context){
  //EXIT ACTION LAYER
  action_bar_layer_remove_from_window(action_bar_layer);
  window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider);
}

static void action_bar_click_config_provider(void *context){
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)action_bar_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)action_bar_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)action_bar_select_click_handler);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  action_bar_layer_add_to_window(action_bar_layer, window);
  action_bar_layer_set_click_config_provider(action_bar_layer, action_bar_click_config_provider);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_page > 0) current_page--;
  persist_write_int(PERSIST_KEY_PAGE, current_page);
  request_page(current_page);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  current_page++;
  persist_write_int(PERSIST_KEY_PAGE, current_page);
  request_page(current_page);
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  brightness_off_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BRIGHTNESS_OFF);
  brightness_on_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BRIGHTNESS_ON);
  done_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EXIT);
  skip_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PERCENT);

  text_layer = text_layer_create(bounds);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);

  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  action_bar_layer = action_bar_layer_create();
  (brightness) ? action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_on_image, true) : action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_off_image, true);
  action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_SELECT, done_image, true);
  action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_DOWN, skip_image, true);
}

static void window_unload(Window *window) {
  gbitmap_destroy(done_image);
  gbitmap_destroy(brightness_on_image);
  gbitmap_destroy(brightness_off_image);
  gbitmap_destroy(skip_image);
  text_layer_destroy(text_layer);
  action_bar_layer_destroy(action_bar_layer);
}

static void init() {
  window = window_create();

  if (persist_exists(PERSIST_KEY_PAGE)) {
    current_page = persist_read_int(PERSIST_KEY_PAGE);
  }
  if (persist_exists(PERSIST_KEY_BRIGHTNESS)){
    brightness = persist_read_bool(PERSIST_KEY_BRIGHTNESS);
  }
  light_enable(brightness);

  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  window_stack_push(window, true);

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(4096, 4096);

  request_page(current_page);
}

static void deinit() {
  light_enable(false);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
