#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
static Window *s_main_window;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
static GFont s_time_font;
static GFont s_standard_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char buffer[] ="00/00/00";
  strftime(buffer, sizeof("00/00/00"), "%D", tick_time);
  text_layer_set_text(s_date_layer, buffer);
}


static void update_time() {
	// temporary struct
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	// create lifetime buffer
	static char buffer[] = "00:00";
	
	// current hours and minutes go into the buffer
	if(clock_is_24h_style() == true) {
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	}
	else {
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	
	// display
	text_layer_set_text(s_time_layer, buffer);
}

// Time handlers
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
  update_date();
  
  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
}

static void improve_text_layer(TextLayer *the_text_layer, GFont font) {
  text_layer_set_font(the_text_layer, font);
  text_layer_set_text_alignment(the_text_layer, GTextAlignmentCenter);
}

// Window Handlers
static void main_window_load(Window *window) {
  // background
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // date text layer
  s_date_layer = text_layer_create(GRect(0, 10, 144, 40));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // time text layer
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  // weather layer
  s_weather_layer = text_layer_create(GRect(0, 120, 144, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  
  // improve the view
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_VGA_48));
  s_standard_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  improve_text_layer(s_time_layer, s_time_font);
  improve_text_layer(s_date_layer, s_standard_font);
  improve_text_layer(s_weather_layer, s_standard_font);
  
  // add our text layers
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  // displays time upon load
  update_time();
  update_date();

}
static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_time_font);
}
// App messaging
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char temperture_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  // First item
  Tuple *t = dict_read_first(iterator);
  
  // Is the latest key NULL after latest iteration?
  while (t != NULL) {
    switch (t->key) {
      case KEY_TEMPERATURE:
      // write outputted format to sized buffer
      snprintf(temperture_buffer, sizeof(temperture_buffer), "%dF", (int)t->value->int32);
      break;
      
      case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
      
      default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unexpected key:%d", (int)t->key);
    }
    
    // Move onto to the next key
    t = dict_read_next(iterator);
  }
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperture_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

// Setup and breakdown
static void init () {
  // create main window
  s_main_window = window_create();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // set the window handlers
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit () {
  window_destroy(s_main_window);
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

int main(void) {
  init ();
  app_event_loop();
  deinit();
}