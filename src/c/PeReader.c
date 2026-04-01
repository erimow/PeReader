#include <pebble.h>
#include <stdbool.h>
#include "message_keys.auto.h"

static Window *window;
static TextLayer *text_layer;
static TextLayer *time_text_layer;
static ActionBarLayer *action_bar_layer;
static NumberWindow *number_window;

//BMPS
static GBitmap *done_image;
static GBitmap *brightness_on_image;
static GBitmap *brightness_off_image;
static GBitmap *skip_image;

static unsigned int current_page = 0;
static unsigned int total_pages = 0;

static bool brightness = false;
static bool actionbar_enabled = false;

#define PERSIST_KEY_PAGE 1
#define PERSIST_KEY_BRIGHTNESS 50

static void click_config_provider(void *context);

static void update_time() {
  static char buffer[] = "00:00";
  if (actionbar_enabled){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);


    if(clock_is_24h_style()) {
      strftime(buffer, sizeof(buffer), "%H:%M", tick_time);
    } else {
      strftime(buffer, sizeof(buffer), "%I:%M", tick_time);
    }
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Buffer: %s\n", buffer);
  text_layer_set_text(time_text_layer, buffer);
  }else{
    text_layer_set_text(time_text_layer, "");
  }

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void update_display(const char *text) {
  static char buffer[512];

  snprintf(buffer, sizeof(buffer), "Page %u/%u\n%s",
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

static void number_window_select_callback(struct NumberWindow *number_window, void *context){
  current_page = (total_pages*((float)number_window_get_value(number_window)/100));
  persist_write_int(PERSIST_KEY_PAGE, current_page);
  window_stack_pop(true);
  // window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider); //redundant
  request_page(current_page);
}

static void action_bar_down_click_handler(ClickRecognizerRef recognizer, void *context){
  //SKIP SECTIONS OPTION
  window_stack_push(number_window_get_window(number_window), true);
  action_bar_layer_remove_from_window(action_bar_layer);
  window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider);
  actionbar_enabled=false;
  update_time();
  // window_set_click_config_provider(window, window_get_click_config_provider(number_window_get_window(number_window)));// redundant
}
static void action_bar_up_click_handler(ClickRecognizerRef recognizer, void *context){
  //BACKLIGHT ON
  brightness = !brightness;
  persist_write_bool(PERSIST_KEY_BRIGHTNESS, brightness);
  (brightness) ? action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_on_image, true) : action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_off_image, true);
  light_enable(brightness);
}
static void action_bar_select_click_handler(ClickRecognizerRef recognizer, void *context){
  //EXIT ACTION LAYER - also called when pressing the back button
  action_bar_layer_remove_from_window(action_bar_layer);
  window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider);
  actionbar_enabled=false;
  update_time();
}

static void action_bar_click_config_provider(void *context){
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)action_bar_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)action_bar_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)action_bar_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler)action_bar_select_click_handler);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  action_bar_layer_add_to_window(action_bar_layer, window);
  action_bar_layer_set_click_config_provider(action_bar_layer, action_bar_click_config_provider);
  actionbar_enabled = true;
  update_time();
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

static void back_click_handler(ClickRecognizerRef recognizer, void *contest){
  window_stack_pop(true);
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
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

  time_text_layer = text_layer_create(GRect(0,0, bounds.size.w-30, 70));
  text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
  text_layer_set_font(time_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_overflow_mode(time_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(time_text_layer, GColorClear);
  // text_layer_set_text_color(time_text_layer, GColorBlack);

  layer_add_child(window_layer, text_layer_get_layer(time_text_layer));

  action_bar_layer = action_bar_layer_create();
  (brightness) ? action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_on_image, true) : action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_UP, brightness_off_image, true);
  action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_SELECT, done_image, true);
  action_bar_layer_set_icon_animated(action_bar_layer, BUTTON_ID_DOWN, skip_image, true);

  number_window = number_window_create("Skip Percentage", (NumberWindowCallbacks){.selected = number_window_select_callback}, NULL);
  number_window_set_min(number_window, 0);
  number_window_set_max(number_window, 100);
}

static void window_unload(Window *window) {
  gbitmap_destroy(done_image);
  gbitmap_destroy(brightness_on_image);
  gbitmap_destroy(brightness_off_image);
  gbitmap_destroy(skip_image);
  text_layer_destroy(text_layer);
  text_layer_destroy(time_text_layer);
  action_bar_layer_destroy(action_bar_layer);
  number_window_destroy(number_window);
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

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

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
