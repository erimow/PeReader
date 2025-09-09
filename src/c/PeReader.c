#include <pebble.h>
#include "message_keys.auto.h"

#define PERSIST_BOOKMARK_KEY 88

typedef struct {
char *text;
}Page;

static Window *s_window;
static TextLayer *s_text_layer;
static TextLayer *s_pagecount_text_layer;

static char *s_text_buffer = NULL;
static uint16_t s_page = 0;
static uint16_t s_total_pages = 0;
static uint8_t s_words_per_page = 25;
static char **s_words = NULL;
static char s_page_buffer[512];
static unsigned int s_word_count = 1;
static char s_page_number_buffer[10] = "";
//
// #define KEY_TEXT 0
// #define KEY_COMMAND 1

// static void inbox_received_callback(DictionaryIterator *iter, void *context) {
//   Tuple *text_tuple = dict_find(iter, KEY_TEXT);
//   if (text_tuple) {
//     APP_LOG(APP_LOG_LEVEL_DEBUG, "Got text: %s", text_tuple->value->cstring);
//     text_layer_set_text(s_text_layer, text_tuple->value->cstring);
//   } else {
//     APP_LOG(APP_LOG_LEVEL_DEBUG, "No KEY_TEXT tuple received!");
//   }
// }

// static void send_command(uint8_t cmd) {
//   DictionaryIterator *out_iter;
//   if (app_message_outbox_begin(&out_iter) == APP_MSG_OK) {
//     dict_write_uint8(out_iter, KEY_COMMAND, cmd);
//     app_message_outbox_send();
//   }
// }
//

static void save_state(void){
  persist_write_int(PERSIST_BOOKMARK_KEY, s_page);
}

static void load_state(void){
  s_page = persist_read_int(PERSIST_BOOKMARK_KEY);
  
}

static void update_page_number(void){
  snprintf(s_page_number_buffer, sizeof(s_page_number_buffer), "%u/%u", s_page, s_total_pages);
  text_layer_set_text(s_pagecount_text_layer, s_page_number_buffer);
}

static void show_page() {
  unsigned int start = s_page * s_words_per_page;
  unsigned int end   = (s_page+1) * s_words_per_page;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Page: %d, Start: %d, End: %d",s_page, start, end);
  if (end > s_word_count) end = s_word_count;

  int written = 0;
  s_page_buffer[0] = '\0';

  for (unsigned int i = start; i < end; i++) {
    int n = snprintf(s_page_buffer + written,
                     (int)sizeof(s_page_buffer) - written,
                     "%s%s", s_words[i], (i + 1 < end ? " " : ""));
    if (n < 0) break;
    if (n >= (int)sizeof(s_page_buffer) - written) {  // truncated
      written = (int)sizeof(s_page_buffer) - 1;
      break;
    }
    written += n;
  }

  s_page_buffer[sizeof(s_page_buffer) - 1] = '\0';
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Final text: %s", s_page_buffer);
  text_layer_set_text(s_text_layer, s_page_buffer);
  update_page_number();
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //action menu?
  // text_layer_set_text(s_text_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //back page
  // text_layer_set_text(s_text_layer, "Up down legft right down up left right and help me somewhere please I am curious if I am cursious if I am curious and if I am on the way to move the earlt hfro the girl I love but that is just the thingk, I am nont sure I am allowed to do something like that. Am I on the way to the moon or not.");
  // DictionaryIterator *out_iter;
  // app_message_outbox_begin(&out_iter);
  // dict_write_uint8(out_iter, 1, 0); // 0 = prev page
  // app_message_outbox_send();
  // send_command(0);
  show_page((s_page==0)?s_page:--s_page);
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //next page
  // text_layer_set_text(s_text_layer, "Down if you are\n down\n if\n you are not down again.");
  // DictionaryIterator *out_iter;
  // app_message_outbox_begin(&out_iter);
  // dict_write_uint8(out_iter, 1, 1); // 1 = next page
  // app_message_outbox_send();
  // send_command(1);
  show_page(++s_page);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  // if(persist_exists(PERSIST_BOOKMARK_KEY)){
  //    APP_LOG(APP_LOG_LEVEL_DEBUG, "PERSIST EXISTS");}
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h-14));
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  s_pagecount_text_layer = text_layer_create(GRect(0, bounds.size.h-14, bounds.size.w, 14));
  text_layer_set_text_alignment(s_pagecount_text_layer, GTextAlignmentCenter);

  ResHandle rh = resource_get_handle(RESOURCE_ID_BOOK_TXT);
  size_t size = resource_size(rh);
  size_t portionedSize = 0;
  if (size > 20000) {
    portionedSize=size/10;
  }


     APP_LOG(APP_LOG_LEVEL_DEBUG, "size -> %d portioned size: %d", size, portionedSize);

  s_text_buffer = malloc(portionedSize + 1);
  resource_load(rh, (uint8_t *)s_text_buffer, portionedSize);
  s_text_buffer[portionedSize] = '\0';
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: %s", s_text_buffer);
  //
  //   // Split words
  s_word_count = 0;
  for (char *p = s_text_buffer; *p; p++) if (*p == ' ' || *p == '\n') s_word_count++;
  s_words = malloc(sizeof(char*) * (s_word_count + 1));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Word count: %d", s_word_count);
  s_total_pages = s_word_count/s_words_per_page;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Total pages = %d", s_total_pages);

   unsigned int idx = 0;
  char *p = s_text_buffer;
   while (*p && idx<s_word_count) {
      // Start of a word
  s_words[idx++] = p;

  // Advance until delimiter or end
  while (*p && *p != ' ' && *p != '\n') {
    p++;
  }

  // If we stopped on a delimiter, replace it with '\0'
  if (*p) {
    *p = '\0';
    p++;
  }
}
s_words[idx] = NULL; // end marker
  //

  load_state(); //load page saved


APP_LOG(APP_LOG_LEVEL_DEBUG, "Tokenization complete: %d words", idx);
// APP_LOG(APP_LOG_LEVEL_DEBUG, "Test: %s%s", s_words[0], s_words[1]);

     // Show first page
   unsigned int end = (s_page*s_words_per_page)+s_words_per_page;
   if (end > s_word_count) end = s_word_count;
   // s_page_buffer[0] = "\0";
  int written = 0;

for (unsigned int i = s_page*s_words_per_page; i < end; i++) {
  written += snprintf(s_page_buffer + written, (int)sizeof(s_page_buffer) - written,
                      "%s ", s_words[i]);
  if (written >= (int)sizeof(s_page_buffer)) {
    break; // avoid overflow
  }
}

    s_page_buffer[sizeof(s_page_buffer) - 1] = '\0';
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: %s", s_page_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_pagecount_text_layer));
   text_layer_set_text(s_text_layer, s_page_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  update_page_number();
}

static void prv_window_unload(Window *window) {
  save_state();
  text_layer_destroy(s_text_layer);
  text_layer_destroy(s_pagecount_text_layer);
  free(s_words);
  free(s_text_buffer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  // app_message_register_inbox_received(inbox_received_callback);
  // app_message_open(1024, 1024);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
