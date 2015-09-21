#include <pebble.h>

#define BATTERY_W_STEP 14
#define BATTERY_H_STEP 16
  
static Window *s_main_window;
static TextLayer *s_time_hours_layer;
static TextLayer *s_time_minutes_layer;
static TextLayer *s_battery_layer;
static GFont s_time_font;
static GFont s_battery_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static InverterLayer *s_battery_inverter_layer;
static InverterLayer *s_background_inverter_layer;


static void draw_battery_inverter(int battery_charge){
  /*
  battery_charge is expected to be between 1 and 10
  defined above:
  BATTERY_W_STEP: 14
  BATTERY_H_STEP: 16
  We're using these numbers as the display resolution doesn't lend itself to 10 equal steps... This will leaves us with a border when charge is 100%
  */
  int w = battery_charge * BATTERY_W_STEP;
  //int h = battery_charge * BATTERY_H_STEP;
  
  inverter_layer_destroy(s_battery_inverter_layer);
  s_battery_inverter_layer = inverter_layer_create(GRect(2, (168/2)-20, w, 40));
  layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_battery_inverter_layer));
}

static void handle_battery(BatteryChargeState charge_state) {
  static char batterybuffer[] = "000%";
  snprintf(batterybuffer, sizeof("100%"), "%u%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, batterybuffer);
  draw_battery_inverter(charge_state.charge_percent / 10);

}
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char hoursbuffer[] = "00";
  static char minutesbuffer[] = "00";
  

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(hoursbuffer, sizeof("00"), "%H", tick_time);
  } else {
    //Use 12 hour format
    strftime(hoursbuffer, sizeof("00"), "%I", tick_time);
  }
  strftime(minutesbuffer, sizeof("00"), "%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_hours_layer, hoursbuffer);
  text_layer_set_text(s_time_minutes_layer, minutesbuffer);
}

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO_PNG);
  s_background_layer = bitmap_layer_create(GRect(2, (168/2)-18, 140, 36));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  s_background_inverter_layer = inverter_layer_create(GRect(2, (168/2)-20, 140, 40));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_background_inverter_layer));
  
  // Create time TextLayer
  //s_time_layer = text_layer_create(GRect(0, 168-75, 144, 50));
  s_time_hours_layer = text_layer_create(GRect(5,-15,60,75));
  s_time_minutes_layer = text_layer_create(GRect(80,168-75,60,75));
  text_layer_set_background_color(s_time_hours_layer, GColorClear);
  text_layer_set_text_color(s_time_hours_layer, GColorBlack);
  text_layer_set_text(s_time_hours_layer, "00");
  text_layer_set_background_color(s_time_minutes_layer, GColorClear);
  text_layer_set_text_color(s_time_minutes_layer, GColorBlack);
  text_layer_set_text(s_time_minutes_layer, "00");
  
  // Improve the layout to be more like a watchface
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CLOCK_FONT_64));
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_COMIC_SANS_28));
  text_layer_set_font(s_time_hours_layer, s_time_font);
  text_layer_set_text_alignment(s_time_hours_layer, GTextAlignmentLeft);
 
  text_layer_set_font(s_time_minutes_layer, s_time_font);
  text_layer_set_text_alignment(s_time_minutes_layer, GTextAlignmentRight);

  s_battery_layer = text_layer_create(GRect(144-75,0,70,35));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text(s_battery_layer, "000%");
  text_layer_set_font(s_battery_layer, s_battery_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  
  // Add it as InverterLayer *inverter_layera child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_hours_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_minutes_layer));

  
  // Make sure the time is displayed from the start
  update_time();
  handle_battery(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  // Destroy TextLayer
  text_layer_destroy(s_time_hours_layer);
  text_layer_destroy(s_time_minutes_layer);
  text_layer_destroy(s_battery_layer);
  inverter_layer_destroy(s_battery_inverter_layer);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_battery_font);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & SECOND_UNIT) {
    printf("Entered Tick Handler. Seconds changed. %d", tick_time->tm_sec);
  }
 if(units_changed & MINUTE_UNIT) {
    printf("Entered Tick Handler. Minutes changed. %d", tick_time->tm_min);
  }
  if(units_changed & HOUR_UNIT) {
    printf("Entered Tick Handler. Hours changed. %d", tick_time->tm_hour);
  }
  if(units_changed & DAY_UNIT) {
    printf("Entered Tick Handler. Days changed. %d", tick_time->tm_mday);
  }
  if(units_changed & MONTH_UNIT) {
    printf("Entered Tick Handler. Month changed.");
  }
  if(units_changed & YEAR_UNIT) {
    printf("Entered Tick Handler. Year changed.");
  }  
  update_time();
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(handle_battery);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
