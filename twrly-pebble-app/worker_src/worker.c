#include <pebble_worker.h>
#include <stdio.h>
#include <stdbool.h>

#include "data.h"

/*
Gesture Recognizer for Pebble
=============================

PEBBLE ACCELEROMETER COORDINATE SYSTEM:

* The X axis is horizontal with respect to the screen, increasing toward the right.
* The Y axis is vertical with respect to the screen, increasing toward the top.
* The Z axis is normal to the screen, increasing as it comes out of the screen.

State symbols:

    > is X+
    < is X-
    ^ is Y+
    v is Y-
    o is Z+
    x is Z-

So the gesture "><xo" is triggered when the user moves their hand forward, backward, down, then up.
*/

// history management
#define HISTORY_LENGTH 8
char HISTORY[HISTORY_LENGTH] = "........";
void add_history(char state) {
    for (uint8_t i = HISTORY_LENGTH - 1; i > 0; i --) HISTORY[i] = HISTORY[i - 1];
    HISTORY[0] = state;
};
void clear_history(void) {
    for (uint8_t i = 0; i < HISTORY_LENGTH; i ++) HISTORY[i] = '.';
}

void activate_gesture(uint16_t gesture_type) {
    worker_launch_app();
    AppWorkerMessage msg_data = { .data0 = HISTORY[0], .data1 = HISTORY[1], .data2 = HISTORY[2] };
    app_worker_send_message(gesture_type, &msg_data);
}

time_t last_transition_time = 0;
void process_accelerometer_data(const int x, const int y, const int z) {
    const int TRIGGER_THRESHOLD = 600; // acceleration activation level - all levels at or above this are considered active
    const int IGNORE_THRESHOLD = 400; // acceleration non-activation level - all levels at or below this are considered inactive
    const long int RESET_DELAY = 3; // delay in s after which holding the watch still will reset the current gesture
    
    uint16_t ms = 0;
    time_t current_time = 0;
    time_ms(&current_time, &ms);
    
    int abs_x = x >= 0 ? x : -x, abs_y = y >= 0 ? y : -y, abs_z = z >= 0 ? z : -z;
    APP_LOG(APP_LOG_LEVEL_INFO, "%d %d %d", x, y, z);
    if (x >= TRIGGER_THRESHOLD && abs_y <= IGNORE_THRESHOLD && abs_z <= IGNORE_THRESHOLD && HISTORY[0] != '>' && HISTORY[0] != '<') {
        add_history('>'); last_transition_time = current_time;
    } else if (x <= -TRIGGER_THRESHOLD && abs_y <= IGNORE_THRESHOLD && abs_z <= IGNORE_THRESHOLD && HISTORY[0] != '<' && HISTORY[0] != '>') {
        add_history('<'); last_transition_time = current_time;
    } else if (y >= TRIGGER_THRESHOLD && abs_x <= IGNORE_THRESHOLD && abs_z <= IGNORE_THRESHOLD && HISTORY[0] != '^' && HISTORY[0] != 'v') {
        add_history('^'); last_transition_time = current_time;
    } else if (y <= -TRIGGER_THRESHOLD && abs_x <= IGNORE_THRESHOLD && abs_z <= IGNORE_THRESHOLD && HISTORY[0] != 'v' && HISTORY[0] != '^') {
        add_history('v'); last_transition_time = current_time;
    } else if (z >= TRIGGER_THRESHOLD && abs_x <= IGNORE_THRESHOLD && abs_y <= IGNORE_THRESHOLD && HISTORY[0] != 'o' && HISTORY[0] != 'x') {
        add_history('o'); last_transition_time = current_time;
    } else if (z <= -TRIGGER_THRESHOLD && abs_x <= IGNORE_THRESHOLD && abs_y <= IGNORE_THRESHOLD && HISTORY[0] != 'x' && HISTORY[0] != 'o') {
        add_history('x'); last_transition_time = current_time;
    } else if (current_time - last_transition_time >= RESET_DELAY) { // transition back to the start state after a timeout
        clear_history(); last_transition_time = current_time;
    }
    
    for (uint16_t i = 0; i < GESTURES_COUNT; i ++) { // check if the history matches any gestures
        int length = strlen(GESTURES[i]);
        bool matches = 1;
        for (int j = 0; j < length; j ++)
            if (GESTURES[i][j] != HISTORY[length - 1 - j])
                matches = 0;
        if (matches) {
            activate_gesture(i);
            clear_history();
            break;
        }
    }
}

static void data_handler(AccelData *data, uint32_t num_samples) {
    // average out the acceleration values (we don't have to worry about overflow because data values are bounded to [-4000, 4000] and the number of samples is small)
    int32_t x = 0, y = 0, z = 0;
    for (int i = 0; i < (int)num_samples; i ++) { x += data[i].x; y += data[i].y; z += data[i].z; }
    x /= num_samples; y /= num_samples; z /= num_samples;
    //process_accelerometer_data((int)x, (int)y, (int)z);
    process_accelerometer_data(data[0].x, data[0].y - 900, data[0].z + 400); // compensate for accelerometer bias
}

static void worker_init() {
    accel_data_service_subscribe(3, data_handler); // take 3 samples per call of the handler
    accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
}

static void worker_deinit() {
    accel_data_service_unsubscribe();
}

int main(void) {
    worker_init();
    worker_event_loop();
    worker_deinit();
}
