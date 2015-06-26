#include "pebble.h"
#include "hands.h"
#include "elements.h"
#include "libs/pebble-assist.h"
#ifdef PBL_COLOR
	#include "gcolor_definitions.h"
#endif

static void update_date() {
	time_t epoch = time(NULL);
	struct tm *t = localtime(&epoch);
	
	print_time(s_date_buffer, "%d", t);
	
	text_layer_set_text(s_date_label, s_date_buffer);
}

//Updates all the hands on the screen
static void update_hands(Layer *layer, GContext *ctx) {
	time_t epoch = time(NULL);
	struct tm *t = localtime(&epoch);
	
	//These are what allow the hands to rotate. You could figure the math out or just copy paste for minute, hour, seconds, respectively.
	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	gpath_rotate_to(s_second_hand, TRIG_MAX_ANGLE * t->tm_sec / 60);
	gpath_rotate_to(s_second_head, TRIG_MAX_ANGLE * t->tm_sec / 60);
	
	//These sections decide which colors to use and if we are filling the shape, outlining it, or both.
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	#else
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	#endif
	
	gpath_draw_filled(ctx, s_minute_arrow);
	gpath_draw_outline(ctx, s_minute_arrow);
	
	gpath_draw_filled(ctx, s_hour_arrow);
	gpath_draw_outline(ctx, s_hour_arrow);
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	
	gpath_draw_filled(ctx, s_second_hand);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	#ifdef PBL_COLOR
		graphics_context_set_stroke_color(ctx, GColorRed);
		graphics_context_set_stroke_width(ctx, 2);
	#else
		graphics_context_set_stroke_color(ctx, GColorBlack);
	#endif
	
	gpath_draw_filled(ctx, s_second_head);
	gpath_draw_outline(ctx, s_second_head);
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	
	graphics_fill_circle(ctx, GPoint(72,84), 5);

	update_date();
}

//Updates the battery. The first thing also updates the date background as it actually reduces code by doing this
static void update_battery(Layer *layer, GContext *ctx) {
	//This is a bit of a wonky thing. I use this to make the battery percentage fall between 9 and 3 on the clock. 100 * .3 + 15 = 45 which is 9.
	//The hand is drawn inverted so that it will go right to left instead of left to right.
	bat = (battery_state_service_peek().charge_percent * .3) + 15;
	
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(105,74,20,20), 0, GCornerNone);

		graphics_context_set_stroke_color(ctx, GColorBlack);
	#else
		graphics_context_set_stroke_color(ctx, GColorBlack);
	#endif
	graphics_draw_rect(ctx, GRect(105,74,20,20));
	
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	#else
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	#endif
	
	//Because of what I did above, I can use the same math used for minutes to determine where the battery hand falls
	gpath_rotate_to(s_battery_arrow, TRIG_MAX_ANGLE * bat / 60);
	
	gpath_draw_filled(ctx, s_battery_arrow);
	gpath_draw_outline(ctx, s_battery_arrow);

	graphics_context_set_fill_color(ctx, GColorBlack);
	
	graphics_fill_circle(ctx, GPoint(72,116), 3);
}

//This allows the screen to be redrawn each time it is called
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	//On single window apps, you can choose any layer you want to mark dirty. The entire window tree gets marked dirty anyway.
	layer_mark_dirty(s_hands_layer);
}

static void main_window_load(Window *window) {
	GRect bounds = window_get_bounds(window);
	GPoint center = grect_center_point(&bounds);
	
	s_date_font = fonts_load_resource_font(RESOURCE_ID_MONOFONT_18);
	
	//These names are taken from hands.h
	s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
	s_second_hand = gpath_create(&SECOND_HAND_POINTS);
	s_second_head = gpath_create(&SECOND_HEAD_POINTS);
	s_battery_arrow = gpath_create(&BATTERY_HAND_POINTS);
	
	//This positions the center of the arrow. Note the battery is not center as that'd be silly
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);
	gpath_move_to(s_second_hand, center);
	gpath_move_to(s_second_head, center);
	gpath_move_to(s_battery_arrow, GPoint(72,116));
	
	s_background_layer = bitmap_layer_create(bounds);
	s_hands_layer = layer_create(bounds);
	s_battery_layer = layer_create(bounds);
	
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
	
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	
	layer_set_update_proc(s_hands_layer, update_hands);
	layer_set_update_proc(s_battery_layer, update_battery);
	
	s_date_label = text_layer_create(GRect(105,71,20,20));
	
	text_layer_set_text_alignment(s_date_label, GTextAlignmentCenter);
	
	text_layer_set_font(s_date_label, s_date_font);
	
	#ifdef PBL_COLOR
		text_layer_set_colors(s_date_label, GColorCobaltBlue, GColorClear);
	#else
		text_layer_set_colors(s_date_label, GColorBlack, GColorClear);
	#endif
	
	bitmap_layer_add_to_window(s_background_layer, window);
	layer_add_to_window(s_battery_layer, window);
	text_layer_add_to_window(s_date_label, window);
	layer_add_to_window(s_hands_layer, window);
	
}

static void main_window_unload(Window *window) {
	fonts_unload_custom_font(s_date_font);
	
	layer_destroy_safe(s_hands_layer);
	layer_destroy_safe(s_battery_layer);
	text_layer_destroy_safe(s_date_label);
	bitmap_layer_destroy_safe(s_background_layer);
	gbitmap_destroy_safe(s_background_bitmap);
	
	gpath_destroy(s_minute_arrow);
	gpath_destroy(s_hour_arrow);
	gpath_destroy(s_second_hand);
	gpath_destroy(s_second_head);
	gpath_destroy(s_battery_arrow);
}

static void init() {
	s_main_window = window_create();
	window_handlers(s_main_window, main_window_load, main_window_unload);
	window_stack_push(s_main_window, true);
	
	//This makes it so we mark the window dirty every second so the watch updates each second.
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}