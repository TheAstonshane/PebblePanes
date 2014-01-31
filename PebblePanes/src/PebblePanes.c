//PebblePanes: time code initially modified from Pebble SDK 2.0 source examples



#include <pebble.h>

static Window *window;
static TextLayer *pane0_layer; //Pane 0 -- Time
static TextLayer *pane1_text_layer; //Pane 1 -- variable
static TextLayer *pane2_text_layer; //Pane 2 -- variable

static BitmapLayer *weather_icon_layer;
static GBitmap *weather_icon_bitmap = NULL;

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
  Layer *window_layer = window_get_root_layer(window); // initialize window first, then each subsequent pane

  // Init pane0_layer // TIME
    pane0_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 144, 50 } });
    text_layer_set_text_color(pane0_layer, GColorWhite); 
    text_layer_set_background_color(pane0_layer, GColorBlack);
    text_layer_set_text_alignment(pane0_layer, GTextAlignmentCenter);
    text_layer_set_font(pane0_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));

    // Ensures time is displayed immediately (will break if NULL tick event accessed).
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    handle_second_tick(current_time, SECOND_UNIT);      // does text_layer_set_text 
    tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

    layer_add_child(window_get_root_layer(window), text_layer_get_layer(pane0_layer));
  
  

  // Init pane1_text_layer // this is the temperature layer for now...

    pane1_text_layer = text_layer_create((GRect) { .origin = { 0, 50 }, .size = { 144, 55 } });
    text_layer_set_text(pane1_text_layer, "14\u00B0 ");
    text_layer_set_text_alignment(pane1_text_layer, GTextAlignmentRight);
    text_layer_set_background_color(pane1_text_layer, GColorBlack);
    text_layer_set_text_color(pane1_text_layer, GColorWhite);
    text_layer_set_font(pane1_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    layer_add_child(window_layer, text_layer_get_layer(pane1_text_layer));



  //weather_icon_layer
    weather_icon_layer = bitmap_layer_create((GRect) { .origin = { 0, 50 }, .size = { 55,55 } });
    bitmap_layer_set_alignment(weather_icon_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, bitmap_layer_get_layer(weather_icon_layer));
  
    weather_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[3]);
    bitmap_layer_set_bitmap(weather_icon_layer, weather_icon_bitmap);





  // Init pane2_text_layer // new features will go here
    pane2_text_layer = text_layer_create((GRect) { .origin = { 0, 105 }, .size = { 144, 168-50-55 } });
    text_layer_set_text(pane2_text_layer, "");
    text_layer_set_text_alignment(pane2_text_layer, GTextAlignmentCenter);
    text_layer_set_background_color(pane2_text_layer, GColorBlack);
    text_layer_set_text_color(pane2_text_layer, GColorWhite);
    text_layer_set_font(pane2_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    layer_add_child(window_layer, text_layer_get_layer(pane2_text_layer));

  
 

}

static void window_unload(Window *window) {
  //app_sync_deinit(&sync);

  text_layer_destroy(pane0_layer);
  text_layer_destroy(pane1_text_layer);
  text_layer_destroy(pane2_text_layer);
  gbitmap_destroy(weather_icon_bitmap);
  bitmap_layer_destroy(weather_icon_layer);
}


static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorWhite);
  window_set_click_config_provider(window, click_config_provider);
  window_set_fullscreen(window, true);
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
