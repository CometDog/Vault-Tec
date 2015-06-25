static Window *s_main_window;
static TextLayer *s_date_label;
static Layer *s_hands_layer, *s_battery_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_date_font;
static GPath *s_minute_arrow, *s_hour_arrow, *s_second_hand, *s_second_head, *s_battery_arrow;

static char s_date_buffer[] = "DD";

static int8_t bat;