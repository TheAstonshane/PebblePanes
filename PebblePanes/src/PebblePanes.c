#include "pebble.h"

static Window *window;

static TextLayer *pane0_text_layer;
static TextLayer *pane00_text_layer;
static TextLayer *pane1_text_layer;
static TextLayer *pane2_text_layer;
static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static AppSync sync;
static uint8_t sync_buffer[64];

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_TYPE_KEY = 0x2,         // TUPLE_CSTRING
};

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_CLOUDY,                  //0
  RESOURCE_ID_IMAGE_HEAVY_RAIN,             //1
  RESOURCE_ID_IMAGE_LIGHT_RAIN,             //2
  RESOURCE_ID_IMAGE_SNOW,                   //3
  RESOURCE_ID_IMAGE_LIGHTNING,              //4
  RESOURCE_ID_IMAGE_MOSTLY_CLOUDY,          //5
  RESOURCE_ID_IMAGE_SUN,                    //6
  RESOURCE_ID_IMAGE_MOON,                    //7
  RESOURCE_ID_IMAGE_WIND,                   //8
  RESOURCE_ID_IMAGE_OTHER                  //9
};

static void send_cmd(void);

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case WEATHER_ICON_KEY:
      if (icon_bitmap) {
        gbitmap_destroy(icon_bitmap);
      }
      icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
      break;

    case WEATHER_TEMPERATURE_KEY:
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      text_layer_set_text(pane1_text_layer, new_tuple->value->cstring);
      break;

    case WEATHER_TYPE_KEY:
      text_layer_set_text(pane2_text_layer, new_tuple->value->cstring);
      break;
  }
}

////////////////////////////////////////////////////////////////////////
static void update_weather(){
   Tuplet initial_values[] = {
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, "n/a"),
    TupletCString(WEATHER_TYPE_KEY, "n/a"),
  };

   app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}

//Pane loading////////////////////////////////////////////////////////////////////////
static void pane0_text_load(TextLayer *pane0_text_layer, Layer *window_layer) {
  //text_layer_set_text(pane0_text_layer, "tmp");
  text_layer_set_text_color(pane0_text_layer, GColorWhite); 
  text_layer_set_background_color(pane0_text_layer, GColorClear);
  text_layer_set_text_alignment(pane0_text_layer, GTextAlignmentCenter);
  text_layer_set_font(pane0_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(pane0_text_layer));
}

static void pane00_text_load(TextLayer *pane00_text_layer, Layer *window_layer) {
  //text_layer_set_text(pane00_text_layer, "Test");
  text_layer_set_text_alignment(pane00_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(pane00_text_layer, GColorClear);
  text_layer_set_text_color(pane00_text_layer, GColorWhite);
  text_layer_set_font(pane00_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(pane00_text_layer));

  //we don't want this layer to show up all of the time, only once the up button has been pressed
  layer_set_hidden((Layer*) pane00_text_layer, true);
}
static void pane1_text_load(TextLayer *pane1_text_layer, Layer *window_layer) {
  //text_layer_set_text(pane1_text_layer, "14\u00B0 ");
  text_layer_set_text_alignment(pane1_text_layer, GTextAlignmentRight);
  text_layer_set_background_color(pane1_text_layer, GColorClear);
  text_layer_set_text_color(pane1_text_layer, GColorWhite);
  text_layer_set_font(pane1_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  layer_add_child(window_layer, text_layer_get_layer(pane1_text_layer));
}

static void pane2_text_load(TextLayer *pane2_text_layer, Layer *window_layer) {
  //text_layer_set_text(pane2_text_layer, "");
  text_layer_set_text_alignment(pane2_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(pane2_text_layer, GColorClear);
  text_layer_set_text_color(pane2_text_layer, GColorWhite);
  text_layer_set_font(pane2_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  layer_add_child(window_layer, text_layer_get_layer(pane2_text_layer));
}

//updating of time / date stuff//////////////////////////////////////////////////////////
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_minute_tick: setting time in pane0" );
    //set pane0 - time
    static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.
    strftime(time_text, sizeof(time_text), "%l:%M", tick_time);
    text_layer_set_text(pane0_text_layer, time_text);
}

static void handle_hour_update(struct tm* tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "hour_update(): updating weather automatically..." );
  update_weather();
}

static void handle_date_update(struct tm* tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "date_update(): setting date in pane00" );

  static char time_text2[] = "###  ### ##"; // Needs to be static because it's used by the system later.
  strftime(time_text2, sizeof(time_text2), "%a %b %e", tick_time);
  if(time_text2[7] == ' '){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "resizing date in pane00 due to single-digit date" );
    strftime(time_text2, sizeof(time_text2), "%a  %b %e", tick_time);
   }
  text_layer_set_text(pane00_text_layer, time_text2);
}

static void update_time(){ 
  
  //Ensures time is displayed immediately (will break if NULL tick event accessed).
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);

  handle_date_update(current_time, DAY_UNIT);      // does text_layer_set_text 
  tick_timer_service_subscribe(DAY_UNIT, &handle_date_update);

  handle_hour_update(current_time, HOUR_UNIT);
  tick_timer_service_subscribe(HOUR_UNIT, &handle_hour_update);

  handle_minute_tick(current_time, MINUTE_UNIT);      // does text_layer_set_text 
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//click handlers////////////////////////////////////////////////////////////////////////
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
   update_weather();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!layer_get_hidden((Layer*) pane0_text_layer)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "up click handler pressed, pane0_text_layer will now be hidden..." );
    layer_set_hidden((Layer*) pane0_text_layer, true);
    layer_set_hidden((Layer*) pane00_text_layer, false);
  }else{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "up click handler pressed, pane0_text_layer will now be UN-hidden..." );
    layer_set_hidden((Layer*) pane0_text_layer, false);
    layer_set_hidden((Layer*) pane00_text_layer, true);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
////////////////////////////////////////////////////////////////////////////////////////

static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  icon_layer = bitmap_layer_create(GRect(5, 50, 55, 55));
  layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));

  // Init pane0_layer // TIME
  pane0_text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 144, 50 } });
  pane0_text_load(pane0_text_layer, window_layer);

  // Init pane00_layer // date
  pane00_text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 144, 50 } });
  pane00_text_load(pane00_text_layer, window_layer);

  update_time();

  //pane1_text_layer = text_layer_create(GRect(0, 50, 144, 55));
  pane1_text_layer = text_layer_create(GRect(0, 50, 135, 55));

  pane1_text_load(pane1_text_layer, window_layer);

  pane2_text_layer = text_layer_create(GRect(0, 105, 144, 168-50-55));
  pane2_text_load(pane2_text_layer, window_layer);

  update_weather();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  if (icon_bitmap) {
    gbitmap_destroy(icon_bitmap);
  }

  text_layer_destroy(pane2_text_layer);
  text_layer_destroy(pane1_text_layer);
  text_layer_destroy(pane00_text_layer);
  text_layer_destroy(pane0_text_layer);
  bitmap_layer_destroy(icon_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
