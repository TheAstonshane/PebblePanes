//PebblePanes: time code initially modified from Pebble SDK 2.0 source examples



#include <pebble.h>

static Window *window;
static TextLayer *pane0_layer; //Pane 0 -- Time
static TextLayer *pane1_layer; //Pane 1 -- variable
static TextLayer *pane2_layer; //Pane 2 -- variable



static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {

  static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.
  strftime(time_text, sizeof(time_text), "%I:%M", tick_time);
  text_layer_set_text(pane0_layer, time_text);
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
 // GRect bounds = layer_get_bounds(window_layer);

  // Init pane0_layer
    pane0_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 144, 50 } });
    text_layer_set_text_color(pane0_layer, GColorWhite);
    text_layer_set_background_color(pane0_layer, GColorBlack);
    text_layer_set_text_alignment(pane0_layer, GTextAlignmentCenter);
    text_layer_set_font(pane0_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));

    // Ensures time is displayed immediately (will break if NULL tick event accessed).
    // (This is why it's a good idea to have a separate routine to do the update itself.)
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
      // does text_layer_set_text above
    handle_second_tick(current_time, SECOND_UNIT);
    tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

    layer_add_child(window_get_root_layer(window), text_layer_get_layer(pane0_layer));
  
  // Init pane1_layer
    pane1_layer = text_layer_create((GRect) { .origin = { 0, 50 }, .size = { 144, 50 } });
    text_layer_set_text(pane1_layer, "Test");
    text_layer_set_text_alignment(pane1_layer, GTextAlignmentCenter);
    text_layer_set_background_color(pane1_layer, GColorWhite);
    text_layer_set_text_color(pane1_layer, GColorBlack);
    text_layer_set_font(pane1_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    layer_add_child(window_layer, text_layer_get_layer(pane1_layer));

// Init pane2_layer
    pane2_layer = text_layer_create((GRect) { .origin = { 0, 100 }, .size = { 144, 50 } });
    text_layer_set_text(pane2_layer, "Test");
    text_layer_set_text_alignment(pane2_layer, GTextAlignmentCenter);
    text_layer_set_background_color(pane2_layer, GColorBlack);
    text_layer_set_text_color(pane2_layer, GColorWhite);
    text_layer_set_font(pane2_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    layer_add_child(window_layer, text_layer_get_layer(pane2_layer));

  
 

}

static void window_unload(Window *window) {
  text_layer_destroy(pane0_layer);
  text_layer_destroy(pane1_layer);
  text_layer_destroy(pane2_layer);
}


static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
