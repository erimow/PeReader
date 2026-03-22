#include <pebble.h>
#include "message_keys.auto.h"

static Window *window;
static TextLayer *text_layer;

static int current_page = 0;
static int total_pages = 0;

#define PERSIST_KEY_PAGE 1

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

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_page > 0) current_page--;
  persist_write_int(PERSIST_KEY_PAGE, current_page);
  request_page(current_page);
}
static void up_click_handler_long(ClickRecognizerRef recognizer, void *context) {
  current_page-=50;
  if (current_page < 0)
    current_page=0;
  persist_write_int(PERSIST_KEY_PAGE, current_page);
  request_page(current_page);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  current_page++;
  persist_write_int(PERSIST_KEY_PAGE, current_page);
  request_page(current_page);
}
static void down_click_handler_long(ClickRecognizerRef recognizer, void *context) {
  current_page+=50;
  persist_write_int(PERSIST_KEY_PAGE, current_page);
  request_page(current_page);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP, up_click_handler_long);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, down_click_handler_long);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create(bounds);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);

  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init() {
  window = window_create();

  if (persist_exists(PERSIST_KEY_PAGE)) {
    current_page = persist_read_int(PERSIST_KEY_PAGE);
  }

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
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
