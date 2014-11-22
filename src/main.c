#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static GFont s_time_font;
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
  s_date_layer = text_layer_create(GRect(0, 0, 139, 50));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // time text layer
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  // improve the view
  GFont time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_VGA_48));
  GFont date_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  improve_text_layer(s_date_layer, date_font);
  improve_text_layer(s_time_layer, time_font);
  
  // add our text layers
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // displays time upon load
  update_time();
  update_date();

}
static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
  
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
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

int main(void) {
  init ();
  app_event_loop();
  deinit();
}