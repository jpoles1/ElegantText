#include <pebble.h>
#include <math.h>

static Window *main_window;
static TextLayer *hour_layer, *tens_layer, *ones_layer;
static TextLayer *date_layer;
static TextLayer *batt_layer;
static Layer *graph_layer;
//Battery State Holder
static int battery_level;
static bool charging;
//Time references
char *onesMap[13] = {"--", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve"};
char *tensMap[6] = {"o'", "teen", "twenty", "thirty", "fourty", "fifty"};
//Config
bool centered = true;
int houry = 0;
int tensy = 30;
int onesy = 60;
int batterybary = 105;
int batterypcty = 123;
int datey = 120;
//Draw Battery
static void update_battery(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int width = (int)(float)(((float)battery_level / 100.0F) * (float)bounds.size.w);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, bounds, 4, GCornersAll);
  graphics_context_set_fill_color(ctx, charging ? GColorGreen:GColorChromeYellow);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 4, GCornersAll);
}
static void update_battery_pct(){
  static char batt_buffer[8] = "";
  snprintf(batt_buffer, sizeof(batt_buffer), battery_level == 100 ? "100":"%i%%", battery_level);
  text_layer_set_text_color(batt_layer, battery_level < 30 ? GColorRed:GColorWhite);
  text_layer_set_text(batt_layer, batt_buffer);
}
static void battery_handler(BatteryChargeState state){
  battery_level = state.charge_percent;
  charging = state.is_charging;
  layer_mark_dirty(graph_layer);
  update_battery_pct();
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
  //Display Minutes
  int min = tick_time->tm_min;
  int tens = floor(min/10);
  int ones = min%10;
  static char tens_buffer[10];
  static char ones_buffer[10];
  if(tens==0){
    strcpy(tens_buffer, "o'");
    strcpy(ones_buffer, "clock");
  }
  else if(tens == 1){
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
  //Date
  static char date_buffer[10];
  strftime(date_buffer, sizeof(date_buffer), "%a\n%d", tick_time);
  text_layer_set_text(date_layer, date_buffer);
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
  //Date
  date_layer = text_layer_create(
    GRect(10, datey, 40, 40)
  );
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_text(date_layer, "Day\nDoW");
  //Battery %
  batt_layer = text_layer_create(
    GRect(bounds.size.w-50, batterypcty, 40, 40)
  );
  text_layer_set_background_color(batt_layer, GColorClear);
  text_layer_set_text_color(batt_layer, GColorWhite);
  text_layer_set_text(batt_layer, "XX %");
  //Setup Fonts
  text_layer_set_font(hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(tens_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(ones_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(batt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  //Optional text centering
  if(centered){
    text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(tens_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(ones_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(batt_layer, GTextAlignmentCenter);
  }
  //Add text layers to Window
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(tens_layer));
  layer_add_child(window_layer, text_layer_get_layer(ones_layer));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(batt_layer));
  //Draw Graphics
  graph_layer = layer_create(GRect(5, batterybary, bounds.size.w-10, 8));
  layer_set_update_proc(graph_layer, update_battery);
  //Add to Window
  layer_add_child(window_get_root_layer(window), graph_layer);
}
static void main_window_unload(Window *window){
  text_layer_destroy(hour_layer);
  text_layer_destroy(tens_layer);
  text_layer_destroy(ones_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(batt_layer);
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
  battery_state_service_subscribe(battery_handler);
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
