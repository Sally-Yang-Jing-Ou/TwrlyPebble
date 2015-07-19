#include <pebble.h>

static Window *main_window;
static TextLayer *info_layer;
static BitmapLayer *background_layer;
static GBitmap *background_bitmap;

bool check_appmessage_result(AppMessageResult result) {
    switch (result) {
        case APP_MSG_OK: return true;
        case APP_MSG_SEND_TIMEOUT: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_SEND_TIMEOUT"); return false;
        case APP_MSG_SEND_REJECTED: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_SEND_REJECTED"); return false;
        case APP_MSG_NOT_CONNECTED: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_NOT_CONNECTED"); return false;
        case APP_MSG_APP_NOT_RUNNING: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_APP_NOT_RUNNING"); return false;
        case APP_MSG_INVALID_ARGS: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_INVALID_ARGS"); return false;
        case APP_MSG_BUSY: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_BUSY"); return false;
        case APP_MSG_BUFFER_OVERFLOW: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_BUFFER_OVERFLOW"); return false;
        case APP_MSG_ALREADY_RELEASED: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_ALREADY_RELEASED"); return false;
        case APP_MSG_CALLBACK_ALREADY_REGISTERED: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_CALLBACK_ALREADY_REGISTERED"); return false;
        case APP_MSG_CALLBACK_NOT_REGISTERED: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_CALLBACK_NOT_REGISTERED"); return false;
        case APP_MSG_OUT_OF_MEMORY: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_OUT_OF_MEMORY"); return false;
        case APP_MSG_CLOSED: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_CLOSED"); return false;
        case APP_MSG_INTERNAL_ERROR: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: APP_MSG_INTERNAL_ERROR"); return false;
        default: APP_LOG(APP_LOG_LEVEL_ERROR, "check_appmessage_result: UNKNOWN MESSAGING ERROR"); return false;
    }
}

static void on_send_success(DictionaryIterator *sent, void *context) { }
static void on_send_failed(DictionaryIterator *failed, AppMessageResult reason, void* context) { check_appmessage_result(reason); }
void activate_gesture(int16_t gesture_type) {
	vibes_short_pulse();
    app_message_register_outbox_sent(on_send_success);
    app_message_register_outbox_failed(on_send_failed);
    app_message_open(64, 64);
    DictionaryIterator *iter;
    if (!check_appmessage_result(app_message_outbox_begin(&iter))) return;
    static uint8_t outbox_buffer[50];
    switch (dict_write_begin(iter, outbox_buffer, 50)) {
        case DICT_OK: break;
        case DICT_INVALID_ARGS: APP_LOG(APP_LOG_LEVEL_ERROR, "send_alert_to_phone: DICT_INVALID_ARGS"); return;
        case DICT_INTERNAL_INCONSISTENCY: APP_LOG(APP_LOG_LEVEL_ERROR, "send_alert_to_phone: DICT_INTERNAL_INCONSISTENCY"); return;
        case DICT_MALLOC_FAILED: APP_LOG(APP_LOG_LEVEL_ERROR, "send_alert_to_phone: DICT_MALLOC_FAILED"); return;
        case DICT_NOT_ENOUGH_STORAGE: APP_LOG(APP_LOG_LEVEL_ERROR, "send_alert_to_phone: DICT_NOT_ENOUGH_STORAGE"); return;
    }
    dict_write_uint16(iter, 0x0, gesture_type);
    if (dict_write_end(iter) == 0) { APP_LOG(APP_LOG_LEVEL_ERROR, "send_alert_to_phone: ERROR ENDING DICTIONARY"); return; }
    check_appmessage_result(app_message_outbox_send());
}

// worker functions
static void launch_background_worker(void) {
    if (app_worker_is_running()) return; // already running
    switch (app_worker_launch()) {
        case APP_WORKER_RESULT_SUCCESS: break;
        case APP_WORKER_RESULT_NO_WORKER: APP_LOG(APP_LOG_LEVEL_ERROR, "launch_background_worker: APP_WORKER_RESULT_NO_WORKER"); break;
        case APP_WORKER_RESULT_NOT_RUNNING: APP_LOG(APP_LOG_LEVEL_ERROR, "launch_background_worker: APP_WORKER_RESULT_NOT_RUNNING"); break;
        case APP_WORKER_RESULT_ALREADY_RUNNING: APP_LOG(APP_LOG_LEVEL_ERROR, "launch_background_worker: APP_WORKER_RESULT_ALREADY_RUNNING"); break;
        case APP_WORKER_RESULT_DIFFERENT_APP: APP_LOG(APP_LOG_LEVEL_ERROR, "launch_background_worker: APP_WORKER_RESULT_DIFFERENT_APP"); break;
        case APP_WORKER_RESULT_ASKING_CONFIRMATION: APP_LOG(APP_LOG_LEVEL_ERROR, "launch_background_worker: APP_WORKER_RESULT_ASKING_CONFIRMATION"); break;
        default: APP_LOG(APP_LOG_LEVEL_ERROR, "launch_background_worker: UNKNOWN ERROR WHILE LAUNCHING BACKGROUND WORKER"); break;
    }
}
char combined_string[32] = "";
static void worker_message_handler(uint16_t type, AppWorkerMessage *data) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "worker_message_handler: GESTURE TYPE %d DETECTED", type);
    snprintf(combined_string, 32, "%d: %c %c %c", type, (char)data->data0, (char)data->data1, (char)data->data2);
    //snprintf(combined_string, 32, "%d %d %d", (int16_t)data->data0, (int16_t)data->data1, (int16_t)data->data2);
    text_layer_set_text(info_layer, combined_string);
    activate_gesture(type);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window); GRect bounds = layer_get_bounds(window_layer);

    // background image
    background_layer = bitmap_layer_create(layer_get_bounds(window_layer));
    background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    bitmap_layer_set_bitmap(background_layer, background_bitmap);
#ifdef PBL_PLATFORM_APLITE
    bitmap_layer_set_compositing_mode(background_layer, GCompOpAssign);
#elif PBL_PLATFORM_BASALT
    bitmap_layer_set_compositing_mode(background_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
#endif
    
    // info text
    info_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30));
    text_layer_set_text(info_layer, "GESTURES ON");
    text_layer_set_font(info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(info_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(info_layer));

    launch_background_worker();
}
static void main_window_unload(Window *window) {
    text_layer_destroy(info_layer);
}

void init(void) {
    main_window = window_create();
    window_set_background_color(main_window, GColorWhite);
    window_set_window_handlers(main_window, (WindowHandlers) { .load = main_window_load, .unload = main_window_unload });
    window_stack_push(main_window, true);
    app_worker_message_subscribe(worker_message_handler); // register shake handler
}

void deinit(void) {
    app_worker_message_unsubscribe();
	window_destroy(main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
