#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;

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
}

// Window Handlers
static void main_window_load(Window *window) {
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  // improve the view
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // add it as a child to the window root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // displays time upon load
  update_time();

}
static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  
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
  
}

static void deinit () {
  window_destroy(s_main_window);
  
}

int main(void) {
  init ();
  app_event_loop();
  deinit();
}