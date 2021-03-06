/*

  Just A Bit Less watch.
  A "binary" clock.

  Heavily derived from Pebble's Just A Bit watch.
  But I didn't want the seconds to save battery and have the date instead.

 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID {0x95, 0x7F, 0x38, 0xA7, 0x22, 0xAD, 0x49, 0xCF, 0xA9, 0x21, 0x32, 0xB5, 0xC6, 0x89, 0xC1, 0xD2}
PBL_APP_INFO(MY_UUID, "Just A Bit Less", "KMA", 0x1, 0x1, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;

Layer display_layer;
TextLayer text_date_layer;
TextLayer text_day_layer;
TextLayer text_wk_layer;
static char date_text[] = "Xxxxxxxxx 00";
static char day_text[] = "Xxxxxxxxx 00";
static char wk_text[] = "wk00";

#define CIRCLE_RADIUS 12
#define CIRCLE_LINE_THICKNESS 2

void draw_cell(GContext* ctx, GPoint center, bool filled) {
  // Each "cell" represents a binary digit or 0 or 1.

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_fill_circle(ctx, center, CIRCLE_RADIUS);

  if (!filled) {
    // This is our ghetto way of drawing circles with a line thickness
    // of more than a single pixel.
    graphics_context_set_fill_color(ctx, GColorBlack);

    graphics_fill_circle(ctx, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
  }

}


#define CIRCLE_PADDING 14 - CIRCLE_RADIUS // Number of padding pixels on each side
#define CELL_SIZE (2 * (CIRCLE_RADIUS + CIRCLE_PADDING)) // One "cell" is the square that contains the circle.
#define SIDE_PADDING (144 - (4 * CELL_SIZE))/2

#define CELLS_PER_ROW 4
#define CELLS_PER_COLUMN 4 // changed from 6 to 4


GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
  // Cell location (0,0) is upper left, location (4, 6) is lower right.
  return GPoint(SIDE_PADDING + (CELL_SIZE/2) + (CELL_SIZE * x),
		(CELL_SIZE/2) + (CELL_SIZE * y));
}

void draw_cell_row_for_digit(GContext* ctx, unsigned short digit, unsigned short max_columns_to_display, 
							 unsigned short cell_row) {
  // Converts the supplied decimal digit into Binary Coded Decimal form and
  // then draws a row of cells on screen--'1' binary values are filled, '0' binary values are not filled.
  // `max_columns_to_display` restricts how many binary digits are shown in the row.
  for (int cell_column_index = 0; cell_column_index < max_columns_to_display; cell_column_index++) {
    draw_cell(ctx, get_center_point_from_cell_location(cell_column_index, cell_row), 
			  (digit >> cell_column_index) & 0x1);
  }
}


// The cell row offsets for each digit
#define HOURS_FIRST_DIGIT_ROW 0
#define HOURS_SECOND_DIGIT_ROW 1
#define MINUTES_FIRST_DIGIT_ROW 2
#define MINUTES_SECOND_DIGIT_ROW 3
// #define SECONDS_FIRST_DIGIT_ROW 4
// #define SECONDS_SECOND_DIGIT_ROW 5

// The maximum number of cell columns to display
// (Used so that if a binary digit can never be 1 then no un-filled
// placeholder is shown.)
#define DEFAULT_MAX_COLS 4
#define HOURS_FIRST_DIGIT_MAX_COLS 2
#define MINUTES_FIRST_DIGIT_MAX_COLS 3
// #define SECONDS_FIRST_DIGIT_MAX_COLS 3


unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}


void display_layer_update_callback(Layer *me, GContext* ctx) {

  PblTm t;

  get_time(&t);

  unsigned short display_hour = get_display_hour(t.tm_hour);

  draw_cell_row_for_digit(ctx, display_hour / 10, HOURS_FIRST_DIGIT_MAX_COLS, HOURS_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, display_hour % 10, DEFAULT_MAX_COLS, HOURS_SECOND_DIGIT_ROW);

  draw_cell_row_for_digit(ctx, t.tm_min / 10, MINUTES_FIRST_DIGIT_MAX_COLS, MINUTES_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t.tm_min % 10, DEFAULT_MAX_COLS, MINUTES_SECOND_DIGIT_ROW);

//  draw_cell_row_for_digit(ctx, t.tm_sec / 10, SECONDS_FIRST_DIGIT_MAX_COLS, SECONDS_FIRST_DIGIT_ROW);
//  draw_cell_row_for_digit(ctx, t.tm_sec % 10, DEFAULT_MAX_COLS, SECONDS_SECOND_DIGIT_ROW);
	
}

void update_watchface(PblTm* t) {
	
    string_format_time(date_text, sizeof(date_text), "%B %e", t);
    text_layer_set_text(&text_date_layer, date_text);
	string_format_time(day_text, sizeof(day_text), "%A", t);
	text_layer_set_text(&text_day_layer, day_text);
	string_format_time(wk_text, sizeof(day_text), "wk%V", t);
	text_layer_set_text(&text_wk_layer, wk_text);
	
}

void handle_init(AppContextRef ctx) {

  window_init(&window, "Just A Bit Less watch");
  window_stack_push(&window, true);

  window_set_background_color(&window, GColorBlack);
  
  resource_init_current_app(&APP_RESOURCES);
  
  // Init the layer for the display
  layer_init(&display_layer, window.layer.frame);
  display_layer.update_proc = &display_layer_update_callback;
  layer_add_child(&window.layer, &display_layer);

  text_layer_init(&text_date_layer, window.layer.frame);
  text_layer_set_text_color(&text_date_layer, GColorWhite);
  text_layer_set_background_color(&text_date_layer, GColorClear);
  layer_set_frame(&text_date_layer.layer, GRect(8, 112, 144-8, 168-112));
  text_layer_set_font(&text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&window.layer, &text_date_layer.layer);

  text_layer_init(&text_day_layer, window.layer.frame);
  text_layer_set_text_color(&text_day_layer, GColorWhite);
  text_layer_set_background_color(&text_day_layer, GColorClear);
  layer_set_frame(&text_day_layer.layer, GRect(8, 135, 144-8, 168-135));
  text_layer_set_font(&text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&window.layer, &text_day_layer.layer);

  text_layer_init(&text_wk_layer, window.layer.frame);
  text_layer_set_text_color(&text_wk_layer, GColorWhite);
  text_layer_set_background_color(&text_wk_layer, GColorClear);
  layer_set_frame(&text_wk_layer.layer, GRect(90, 2, 144-4, 23));
  text_layer_set_font(&text_wk_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&window.layer, &text_wk_layer.layer);

  // load watchface immediately
  PblTm t;
  get_time(&t);
  update_watchface(&t);

}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {

  //layer_mark_dirty(&display_layer);
  update_watchface(t->tick_time);

}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
