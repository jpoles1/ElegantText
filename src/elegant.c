#include <pebble.h>
#include <math.h>
#define color_set 0
#define bg_R 1
#define bg_G 2
#define bg_B 3
#define txt_R 4
#define txt_G 5
#define txt_B 6
#define ac_R 7
#define ac_G 8
#define ac_B 9
static Window *main_window;
static TextLayer *hour_layer, *tens_layer, *ones_layer;
static TextLayer *weekday_layer, *date_layer, *month_layer;
static TextLayer *batt_layer;
static Layer *graph_layer;
static Layer *cal_layer;
static GColor bg_color;
static GColor txt_color;
static GColor ac_color;
//Battery State Holder
static int battery_level;
static bool charging;
//Time references
char *onesMap[13] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve"};
char *tensMap[6] = {"o'", "teen", "twenty", "thirty", "forty", "fifty"};
//Config
bool centered = true;
int houry = 0;
int tensy = 30;
int onesy = 60;
int batterybary = 102;
int batterypctx = 96;
int batterypcty = 123;
int datex = 50;
int datey = 122;
//Draw Battery
static void update_battery(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int width = (int)(float)(((float)battery_level / 100.0F) * (float)bounds.size.w);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, bounds, 4, GCornersAll);
  graphics_context_set_fill_color(ctx, charging ? GColorGreen:ac_color);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 4, GCornersAll);
}
static void update_battery_pct(){
  static char batt_buffer[4] = "";
  snprintf(batt_buffer, sizeof(batt_buffer), battery_level == 100 ? "100":"%i%%", battery_level);
  text_layer_set_text_color(batt_layer, battery_level < 30 ? ac_color:txt_color);
  text_layer_set_text(batt_layer, batt_buffer);
}
static void battery_handler(BatteryChargeState state){
  battery_level = state.charge_percent;
  charging = state.is_charging;
  layer_mark_dirty(graph_layer);
  update_battery_pct();
}
static void calendar_box(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, txt_color);
  graphics_draw_rect(ctx, bounds);
}
//Change the time
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  //Display hour
  int hour = tick_time->tm_hour;
  static char hour_buffer[8];
  strcpy(hour_buffer, onesMap[hour%12==0 ? 12:hour%12]);
  text_layer_set_text(hour_layer, hour_buffer);
  //Display Minutes
  int min = tick_time->tm_min;
  int tens = floor(min/10);
  int ones = min%10;
  static char tens_buffer[10];
  static char ones_buffer[10];
  if(tens==0 && ones==0){
    strcpy(tens_buffer, "o'");
    strcpy(ones_buffer, "clock");
  }
  else if(tens == 1){
    switch(ones){
      case 0:
        strcpy(tens_buffer, "ten");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 1:
        strcpy(tens_buffer, "eleven");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 2:
        strcpy(tens_buffer, "twelve");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 3:
        strcpy(tens_buffer, "thirteen");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 4:
        strcpy(tens_buffer, "fourteen");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 5:
        strcpy(tens_buffer, "fifteen");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 6:
        strcpy(tens_buffer, "sixteen");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 7:
        strcpy(tens_buffer, "svnteen");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 8:
        strcpy(tens_buffer, "eighteen");
        strcpy(ones_buffer, onesMap[0]);
        break;
      case 9:
        strcpy(tens_buffer, "nineteen");
        strcpy(ones_buffer, onesMap[0]);
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
  static char weekday_buffer[5];
  static char month_buffer[5];
  static char date_buffer[5];
  strftime(weekday_buffer, sizeof(weekday_buffer), "%a", tick_time);
  strftime(month_buffer, sizeof(month_buffer), "%b", tick_time);
  strftime(date_buffer, sizeof(date_buffer), "%d", tick_time);
  text_layer_set_text(weekday_layer, weekday_buffer);
  text_layer_set_text(month_layer, month_buffer);
  text_layer_set_text(date_layer, date_buffer);
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}
//Load and Unload Window (create and setup resources)
static void main_window_load(Window *window) {
  bg_color = GColorBlack;
  txt_color = GColorWhite;
  ac_color = GColorChromeYellow;
  int persist_set = persist_read_int(color_set);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Persist? - %d", persist_set);
  if(persist_set!=0){
    int red = persist_read_int(bg_R);
    int green = persist_read_int(bg_G);
    int blue = persist_read_int(bg_B);
    bg_color = GColorFromRGB(red, green, blue);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Got Background Color - R: %d, G: %d, B: %d", red, green, blue);
    red = persist_read_int(txt_R);
    green = persist_read_int(txt_G);
    blue = persist_read_int(txt_B);
    txt_color = GColorFromRGB(red, green, blue);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Got Text Color - R: %d, G: %d, B: %d", red, green, blue);
    red = persist_read_int(ac_R);
    green = persist_read_int(ac_G);
    blue = persist_read_int(ac_B);
    ac_color = GColorFromRGB(red, green, blue);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Got Accent Color - R: %d, G: %d, B: %d", red, green, blue);
  }
  //Set window background
  window_set_background_color(main_window, bg_color);
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  //Hours
  hour_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(houry, houry), bounds.size.w, 50)
  );
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_color(hour_layer, ac_color);
  text_layer_set_text(hour_layer, "hour");
  //Tens
  tens_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(tensy, tensy), bounds.size.w, 50)
  );
  text_layer_set_background_color(tens_layer, GColorClear);
  text_layer_set_text_color(tens_layer, txt_color);
  text_layer_set_text(tens_layer, "tens");
  //Ones
  ones_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(onesy, onesy), bounds.size.w, 50)
  );
  text_layer_set_background_color(ones_layer, GColorClear);
  text_layer_set_text_color(ones_layer, txt_color);
  text_layer_set_text(ones_layer, "ones");
  //Day of Week
  weekday_layer = text_layer_create(
    GRect(15, batterypcty, 40, 40)
  );
  text_layer_set_background_color(weekday_layer, GColorClear);
  text_layer_set_text_color(weekday_layer, txt_color);
  text_layer_set_text(weekday_layer, "XX %");
  //Month
  month_layer = text_layer_create(
    GRect(datex, datey-4, 40, 21)
  );
  text_layer_set_background_color(month_layer, txt_color);
  text_layer_set_text_color(month_layer, bg_color);
  text_layer_set_text(month_layer, "Mnth");
  //Date
  date_layer = text_layer_create(
    GRect(datex, datey+16, 40, 40)
  );
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, txt_color);
  text_layer_set_text(date_layer, "Day");
  //Battery %
  batt_layer = text_layer_create(
    GRect(batterypctx, batterypcty, 40, 40)
  );
  text_layer_set_background_color(batt_layer, GColorClear);
  text_layer_set_text_color(batt_layer, txt_color);
  text_layer_set_text(batt_layer, "XX %");
  //Setup Fonts
  text_layer_set_font(hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(tens_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(ones_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(weekday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_font(batt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  //Optional text centering
  if(centered){
    text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(tens_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(ones_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(month_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(batt_layer, GTextAlignmentCenter);
  }
  //Add text layers to Window
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(tens_layer));
  layer_add_child(window_layer, text_layer_get_layer(ones_layer));
  layer_add_child(window_layer, text_layer_get_layer(weekday_layer));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(month_layer));
  layer_add_child(window_layer, text_layer_get_layer(batt_layer));
  //Draw Graphics
  graph_layer = layer_create(GRect(5, batterybary, bounds.size.w-10, 8));
  layer_set_update_proc(graph_layer, update_battery);
  cal_layer = layer_create(GRect(datex, datey-5, 40, 40+5));
  layer_set_update_proc(cal_layer, calendar_box);
  //Add to Window
  layer_add_child(window_get_root_layer(window), cal_layer);
  layer_add_child(window_get_root_layer(window), graph_layer);
  update_time();
}
static void main_window_unload(Window *window){
  text_layer_destroy(hour_layer);
  text_layer_destroy(tens_layer);
  text_layer_destroy(ones_layer);
  text_layer_destroy(weekday_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(month_layer);
  text_layer_destroy(batt_layer);
  layer_destroy(graph_layer);
}
//Get Config
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Color scheme?
  persist_write_int(color_set, 1);
  Tuple *bg_r_t = dict_find(iter, bg_R);
  Tuple *bg_g_t = dict_find(iter, bg_G);
  Tuple *bg_b_t = dict_find(iter, bg_B);
  if(bg_r_t && bg_g_t && bg_b_t) {
    // Apply the color if available
    int red = bg_r_t->value->int32;
    int green = bg_g_t->value->int32;
    int blue = bg_b_t->value->int32;

    // Persist values
    persist_write_int(bg_R, red);
    persist_write_int(bg_G, green);
    persist_write_int(bg_B, blue);
    // Update colors
    GColor bg_color = GColorFromRGB(red, green, blue);
    window_set_background_color(main_window, bg_color);
    text_layer_set_text_color(month_layer, bg_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Background Color - R: %d, G: %d, B: %d", red, green, blue);
  }
  else{APP_LOG(APP_LOG_LEVEL_DEBUG, "Cannot fetch bg color");}
  Tuple *txt_r_t = dict_find(iter, txt_R);
  Tuple *txt_g_t = dict_find(iter, txt_G);
  Tuple *txt_b_t = dict_find(iter, txt_B);
  if(txt_r_t && txt_g_t && txt_b_t) {
    // Apply the color if available
    int red = txt_r_t->value->int32;
    int green = txt_g_t->value->int32;
    int blue = txt_b_t->value->int32;
    // Persist values
    persist_write_int(txt_R, red);
    persist_write_int(txt_G, green);
    persist_write_int(txt_B, blue);
    // Update colors
    GColor txt_color = GColorFromRGB(red, green, blue);
    text_layer_set_text_color(tens_layer, txt_color);
    text_layer_set_text_color(ones_layer, txt_color);
    text_layer_set_text_color(weekday_layer, txt_color);
    text_layer_set_background_color(month_layer, txt_color);
    text_layer_set_text_color(date_layer, txt_color);
    text_layer_set_text_color(batt_layer, txt_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Text Color - R: %d, G: %d, B: %d", red, green, blue);
  }
  else{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cannot fetch text color");
  }
  Tuple *ac_r_t = dict_find(iter, ac_R);
  Tuple *ac_g_t = dict_find(iter, ac_G);
  Tuple *ac_b_t = dict_find(iter, ac_B);
  if(ac_r_t && ac_g_t && ac_b_t) {
    // Apply the color if available
    int red = ac_r_t->value->int32;
    int green = ac_g_t->value->int32;
    int blue = ac_b_t->value->int32;
    // Persist values
    persist_write_int(ac_R, red);
    persist_write_int(ac_G, green);
    persist_write_int(ac_B, blue);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Accent Color - R: %d, G: %d, B: %d", red, green, blue);
    // Update colors
    ac_color = GColorFromRGB(red, green, blue);
    text_layer_set_text_color(hour_layer, ac_color);
  }
  else{APP_LOG(APP_LOG_LEVEL_DEBUG, "Cannot fetch accent color");}
  layer_mark_dirty(cal_layer);
  layer_mark_dirty(graph_layer);
  update_battery_pct();
  update_time();
}
//Init & Deinit
static void init(){
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  //Push window
  window_stack_push(main_window, true);
  //Run initial updates
  update_time();
  battery_handler(battery_state_service_peek());
  //Read from config
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
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
