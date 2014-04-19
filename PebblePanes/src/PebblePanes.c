#include "pebble.h"
////////////////////////////////////////////////////////////////////////////////////
//global definitions
static Window *window;

static TextLayer *time_text_layer;
static TextLayer *date_text_layer;
static TextLayer *temp_text_layer;
static TextLayer *weather_text_layer;
static TextLayer *battery_layer;

static TextLayer *calendar_event_text_layer;
static TextLayer *calendar_location_text_layer;
static TextLayer *calendar_time_text_layer;

static TextLayer *sports_team1_text_layer;
static TextLayer *sports_team2_text_layer;
static TextLayer *sports_time_text_layer;

static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static AppSync sync;
static uint8_t sync_buffer[64];

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_TYPE_KEY = 0x2,         // TUPLE_CSTRING
  CALENDAR_EVENT_KEY = 0x3,       // TUPLE_CSTRING
  CALENDAR_LOCATION_KEY = 0x4,    // TUPLE_CSTRING
  CALENDAR_TIME_KEY = 0x5,        // TUPLE_CSTRING
  SPORTS_TEAM1_KEY = 0x6,
  SPORTS_TEAM2_KEY = 0x7,
  SPORTS_TIME_KEY = 0x8,
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

static bool startup = true;
////////////////////////////////////////////////////////////////////////////////////
//data sync...
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
      text_layer_set_text(temp_text_layer, new_tuple->value->cstring);
      break;

    case WEATHER_TYPE_KEY:
      text_layer_set_text(weather_text_layer, new_tuple->value->cstring);
      break;
    case SPORTS_TEAM1_KEY:
      text_layer_set_text(sports_team1_text_layer, new_tuple->value->cstring);
      break;
    case SPORTS_TEAM2_KEY:
      text_layer_set_text(sports_team2_text_layer, new_tuple->value->cstring);
      break;
    case SPORTS_TIME_KEY:
      text_layer_set_text(sports_time_text_layer, new_tuple->value->cstring);
      break;

  }
}

////////////////////////////////////////////////////////////////////////////////////
//updateing battery, weather...
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

static void update_sports(){
   Tuplet initial_values[] = {
    TupletCString(SPORTS_TEAM1_KEY, "team1"),
    TupletCString(SPORTS_TEAM2_KEY, "team2"),
    TupletCString(SPORTS_TIME_KEY, "time"),
  };

   app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}

static void handle_battery() {
  BatteryChargeState charge_state = battery_state_service_peek();
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

//Pane loading////////////////////////////////////////////////////////////////////////
static void time_text_load(TextLayer *time_text_layer, Layer *window_layer) {
  //text_layer_set_text(time_text_layer, "tmp");
  text_layer_set_text_color(time_text_layer, GColorWhite); 
  text_layer_set_background_color(time_text_layer, GColorClear);
  text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
  text_layer_set_font(time_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_text_layer));
}

static void date_text_load(TextLayer *date_text_layer, Layer *window_layer) {
  text_layer_set_text(date_text_layer, "X");
  text_layer_set_text_alignment(date_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(date_text_layer, GColorClear);
  text_layer_set_text_color(date_text_layer, GColorWhite);
  text_layer_set_font(date_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(date_text_layer));

  //we don't want this layer to show up all of the time, only once the up button has been pressed
  layer_set_hidden((Layer*) date_text_layer, true);
}
static void temp_text_load(TextLayer *temp_text_layer, Layer *window_layer) {
  //text_layer_set_text(temp_text_layer, "14\u00B0 ");
  text_layer_set_text_alignment(temp_text_layer, GTextAlignmentRight);
  text_layer_set_background_color(temp_text_layer, GColorClear);
  text_layer_set_text_color(temp_text_layer, GColorWhite);
  text_layer_set_font(temp_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  layer_add_child(window_layer, text_layer_get_layer(temp_text_layer));
}

static void weather_text_load(TextLayer *weather_text_layer, Layer *window_layer) {
  //text_layer_set_text(weather_text_layer, "");
  text_layer_set_text_alignment(weather_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(weather_text_layer, GColorClear);
  text_layer_set_text_color(weather_text_layer, GColorWhite);
  text_layer_set_font(weather_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  layer_add_child(window_layer, text_layer_get_layer(weather_text_layer));
}

static void battery_layer_load(TextLayer *battery_layer, Layer *window_layer) {
  //text_layer_set_text(battery_layer, "n/a");
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));
  handle_battery();

  //we don't want this layer to show up all of the time, only once the up button has been pressed
  layer_set_hidden((Layer*) battery_layer, true);
}

static void calendar_layers_load(TextLayer *calendar_event_text_layer, TextLayer *calendar_location_text_layer,
                                   TextLayer *calendar_time_text_layer, Layer *window_layer){
  
  text_layer_set_text(calendar_event_text_layer, "event");
  text_layer_set_text_alignment(calendar_event_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(calendar_event_text_layer, GColorClear);
  text_layer_set_text_color(calendar_event_text_layer, GColorWhite);
  text_layer_set_font(calendar_event_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(calendar_event_text_layer));
   layer_set_hidden((Layer*) calendar_event_text_layer, true);

  text_layer_set_text(calendar_location_text_layer, "location location location");
  text_layer_set_text_alignment(calendar_location_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(calendar_location_text_layer, GColorClear);
  text_layer_set_text_color(calendar_location_text_layer, GColorWhite);
  text_layer_set_font(calendar_location_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(calendar_location_text_layer));
   layer_set_hidden((Layer*) calendar_location_text_layer, true);

  text_layer_set_text(calendar_time_text_layer, "time");
  text_layer_set_text_alignment(calendar_time_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(calendar_time_text_layer, GColorClear);
  text_layer_set_text_color(calendar_time_text_layer, GColorWhite);
  text_layer_set_font(calendar_time_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(calendar_time_text_layer));
   layer_set_hidden((Layer*) calendar_time_text_layer, true);
}

static void sports_layers_load(TextLayer *sports_team1_text_layer,TextLayer *sports_team2_text_layer, TextLayer *sports_time_text_layer, Layer *window_layer){
  text_layer_set_text(sports_team1_text_layer, "team1");
  text_layer_set_text_alignment(sports_team1_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(sports_team1_text_layer, GColorClear);
  text_layer_set_text_color(sports_team1_text_layer, GColorWhite);
  text_layer_set_font(sports_team1_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(sports_team1_text_layer));
  layer_set_hidden((Layer*) sports_team1_text_layer, true);

  text_layer_set_text(sports_team2_text_layer, "team2");
  text_layer_set_text_alignment(sports_team2_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(sports_team2_text_layer, GColorClear);
  text_layer_set_text_color(sports_team2_text_layer, GColorWhite);
  text_layer_set_font(sports_team2_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(sports_team2_text_layer));
  layer_set_hidden((Layer*) sports_team2_text_layer, true);

  text_layer_set_text(sports_time_text_layer, "time");
  text_layer_set_text_alignment(sports_time_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(sports_time_text_layer, GColorClear);
  text_layer_set_text_color(sports_time_text_layer, GColorWhite);
  text_layer_set_font(sports_time_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(sports_time_text_layer));
  layer_set_hidden((Layer*) sports_time_text_layer, true);
  
}

static void update_date(struct tm* tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "update_date(): setting date in date_text_layer" );

  static char time_text2[] = "###  ### ##"; // Needs to be static because it's used by the system later.
  strftime(time_text2, sizeof(time_text2), "%a %b %e", tick_time);
  if(time_text2[7] == ' '){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "resizing date in date_text_layer due to single-digit date" );
    strftime(time_text2, sizeof(time_text2), "%a  %b %e", tick_time);
   }
  text_layer_set_text(date_text_layer, time_text2);
}

//updating of time / date stuff//////////////////////////////////////////////////////////
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_minute_tick: setting time in time_text_layer" );
    //set time - time
    static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.
    if(clock_is_24h_style()){
      strftime(time_text, sizeof(time_text), "%H:%M", tick_time);
    }else{
      strftime(time_text, sizeof(time_text), "%l:%M", tick_time);
    }
    text_layer_set_text(time_text_layer, time_text);

    //check ot see if we should update date / weather
    char tmp_text[] = "99";
    strftime(tmp_text, sizeof(tmp_text), "%M", tick_time);
    char tmp_text2[] = "99";
    strftime(tmp_text2, sizeof(tmp_text2), "%l", tick_time);

    //update weather every 30 minutes
    if(atoi(tmp_text) == 0 || atoi(tmp_text) == 30){
      APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_minute_tick: calling update_weather()" );
      update_weather();
    }
     //update date / battery at 12:00, twice a day
    if( (atoi(tmp_text) == 0 && atoi(tmp_text2) == 12)) {
      //startup = false;

      APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_minute_tick: updating date and battery" );
      update_date(tick_time, units_changed);
      handle_battery();
    }
   //APP_LOG(APP_LOG_LEVEL_DEBUG, sizeof(text_layer_get_text(date_text_layer)) );
    //make sure to set date / battery layers on startup
   if(startup){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "startup == true" );
    update_date(tick_time, units_changed);
    startup = false;
   }



}



static void update_time(){ 
  
  //Ensures time is displayed immediately (will break if NULL tick event accessed).
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);

  handle_minute_tick(current_time, MINUTE_UNIT);      // updates time
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//click handlers////////////////////////////////////////////////////////////////////////
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
   update_weather();
   update_sports();
   handle_battery();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!layer_get_hidden((Layer*) time_text_layer)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "up click handler pressed, time_text_layer will now be hidden..." );
    layer_set_hidden((Layer*) time_text_layer, true);
    layer_set_hidden((Layer*) date_text_layer, false);
  }else{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "up click handler pressed, time_text_layer will now be UN-hidden..." );
    layer_set_hidden((Layer*) time_text_layer, false);
    layer_set_hidden((Layer*) date_text_layer, true);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "down_click_handler " );
  
  //if the weather_description is currently vissible, display battery info
  if(!layer_get_hidden((Layer*) weather_text_layer)) {
    layer_set_hidden((Layer*) weather_text_layer, true);
    layer_set_hidden((Layer*) battery_layer, false);
  //else if battery is currently visible, display calendar
  }else if(!layer_get_hidden((Layer*) battery_layer)){
    layer_set_hidden((Layer*) battery_layer, true);
    layer_set_hidden((Layer*) calendar_event_text_layer, false);
    layer_set_hidden((Layer*) calendar_location_text_layer, false);
    layer_set_hidden((Layer*) calendar_time_text_layer, false);
  //else if the calendar is visible, show the sports scores
  }else if(!layer_get_hidden((Layer*) calendar_event_text_layer)){
    layer_set_hidden((Layer*) sports_team1_text_layer, false);
    layer_set_hidden((Layer*) sports_team2_text_layer, false);
    layer_set_hidden((Layer*) sports_time_text_layer, false);
    layer_set_hidden((Layer*) calendar_event_text_layer, true);
    layer_set_hidden((Layer*) calendar_location_text_layer, true);
    layer_set_hidden((Layer*) calendar_time_text_layer, true);
  }
  //else, show weather again
  else{
    layer_set_hidden((Layer*) weather_text_layer, false);
    layer_set_hidden((Layer*) sports_team1_text_layer, true);
    layer_set_hidden((Layer*) sports_team2_text_layer, true);
    layer_set_hidden((Layer*) sports_time_text_layer, true);
  }

}

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

  // Init time_text_layer // TIME
  time_text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 144, 50 } });
  time_text_load(time_text_layer, window_layer);

  // Init date_text_layer // date
  date_text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 144, 50 } });
  date_text_load(date_text_layer, window_layer);

  update_time();

  //temp_text_layer = text_layer_create(GRect(0, 50, 144, 55));
  temp_text_layer = text_layer_create(GRect(0, 50, 135, 55));

  temp_text_load(temp_text_layer, window_layer);

  weather_text_layer = text_layer_create(GRect(0, 105, 144, 168-50-55));
  weather_text_load(weather_text_layer, window_layer);

  battery_layer = text_layer_create(GRect(0, 105, 144, 168-50-55));
  battery_layer_load(battery_layer, window_layer);

  update_weather();

  calendar_event_text_layer = text_layer_create(GRect(0, 105, 144, 21));
  calendar_location_text_layer = text_layer_create(GRect(0, 126, 144, 21));
  calendar_time_text_layer = text_layer_create(GRect(0, 147, 144, 21));
  calendar_layers_load(calendar_event_text_layer, calendar_location_text_layer, calendar_time_text_layer, window_layer);

  sports_team1_text_layer = text_layer_create(GRect(0, 105, 144, 21));
  sports_team2_text_layer = text_layer_create(GRect(0, 126, 144, 21));
  sports_time_text_layer = text_layer_create(GRect(0, 147, 144, 21));
  sports_layers_load(sports_team1_text_layer,sports_team2_text_layer, sports_time_text_layer, window_layer);

  update_sports();
}


static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  if (icon_bitmap) {
    gbitmap_destroy(icon_bitmap);
  }
  text_layer_destroy(sports_team1_text_layer);
  text_layer_destroy(sports_team2_text_layer);
  text_layer_destroy(sports_time_text_layer);
  text_layer_destroy(calendar_time_text_layer);
  text_layer_destroy(calendar_location_text_layer);
  text_layer_destroy(calendar_event_text_layer);
  text_layer_destroy(battery_layer);
  text_layer_destroy(weather_text_layer);
  text_layer_destroy(temp_text_layer);
  text_layer_destroy(date_text_layer);
  text_layer_destroy(time_text_layer);
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
