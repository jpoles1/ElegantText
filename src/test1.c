#include <pebble.h>
#include <math.h>

static Window *main_window;
static TextLayer *hour_layer;
static TextLayer *tens_layer;
static TextLayer *ones_layer;
static Layer *graph_layer;
//Battery State Holder
static int battery_level;
//Time references
char *onesMap[13] = {"--", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve"};
char *tensMap[6] = {"o'", "teen", "twenty", "thirty", "fourty", "fifty"};
//Config
bool centered = true;
int houry = 0;
int tensy = 30;
int onesy = 60;
//Draw Div
static void update_battery(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int width = (int)(float)(((float)battery_level / 100.0F) * (float)bounds.size.w);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, bounds, 4, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 4, GCornersAll);
}
static void battery_handler(BatteryChargeState state){
  battery_level = state.charge_percent;
  layer_mark_dirty(graph_layer);
}
//Change the time
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  //Display hour
  int hour = tick_time->tm_hour;
  static char hour_buffer[8];
  strcpy(hour_buffer, onesMap[hour==12 ? 12:hour%12]);
  text_layer_set_text(hour_layer, hour_buffer);
  //Minutes
  int min = tick_time->tm_min;
  int tens = floor(min/10);
  int ones = min%10;
  static char tens_buffer[10];
  static char ones_buffer[10];
  if(tens == 1){
    switch(ones){
      case 0:
        strcpy(tens_buffer, "ten");
        strcpy(ones_buffer, "");
        break;
      case 1:
        strcpy(tens_buffer, "eleven");
        strcpy(ones_buffer, "");
        break;
      case 2:
        strcpy(tens_buffer, "twelve");
        strcpy(ones_buffer, "");
        break;
      case 3:
        strcpy(tens_buffer, "thirteen");
        strcpy(ones_buffer, "");
        break;
      case 4:
        strcpy(tens_buffer, "fourteen");
        strcpy(ones_buffer, "");
        break;
      case 5:
        strcpy(tens_buffer, "fifteen");
        strcpy(ones_buffer, "");
        break;
      case 6:
        strcpy(tens_buffer, "sixteen");
        strcpy(ones_buffer, "");
        break;
      case 7:
        strcpy(tens_buffer, "svnteen");
        strcpy(ones_buffer, "");
        break;
      case 8:
        strcpy(tens_buffer, "eighteen");
        strcpy(ones_buffer, "");
        break;
      case 9:
        strcpy(tens_buffer, "nineteen");
        strcpy(ones_buffer, "");
        break;
    }
  }
  else{
    strcpy(ones_buffer, onesMap[ones]);
    strcpy(tens_buffer, tensMap[tens]);
  }
  text_layer_set_text(tens_layer, tens_buffer);
  text_layer_set_text(ones_layer, ones_buffer);
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}
//Load and Unload Window (create and setup resources)
static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  //Hours
  hour_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(houry, houry), bounds.size.w, 50)
  );
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_color(hour_layer, GColorChromeYellow);
  text_layer_set_text(hour_layer, "hour");
  //Tens
  tens_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(tensy, tensy), bounds.size.w, 50)
  );
  text_layer_set_background_color(tens_layer, GColorClear);
  text_layer_set_text_color(tens_layer, GColorWhite);
  text_layer_set_text(tens_layer, "tens");
  //Ones
  ones_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(onesy, onesy), bounds.size.w, 50)
  );
  text_layer_set_background_color(ones_layer, GColorClear);
  text_layer_set_text_color(ones_layer, GColorWhite);
  text_layer_set_text(ones_layer, "ones");
  //Setup Fonts
  text_layer_set_font(hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(tens_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(ones_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  //Optional text centering
  if(centered){
    text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(tens_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(ones_layer, GTextAlignmentCenter);
  }
  //Add text layers to Window
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(tens_layer));
  layer_add_child(window_layer, text_layer_get_layer(ones_layer));
  //Draw Graphics
  graph_layer = layer_create(GRect(5, onesy+45, bounds.size.w-10, 8));
  layer_set_update_proc(graph_layer, update_battery);
  //Add to Window
  layer_add_child(window_get_root_layer(window), graph_layer);
}
static void main_window_unload(Window *window){
  text_layer_destroy(hour_layer);
  text_layer_destroy(tens_layer);
  text_layer_destroy(ones_layer);
  layer_destroy(graph_layer);
}
//Init & Deinit
static void init(){
  main_window = window_create();
  window_set_background_color(main_window, GColorBlack);
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  window_stack_push(main_window, true);
  update_time();
  battery_handler(battery_state_service_peek());
}
static void deinit(){
  window_destroy(main_window);
}
//Run Everything
int main(void){
  init();
  app_event_loop();
  deinit();
}
