#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zmk/activity.h>
#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>

static const struct led_dt_spec profile_leds[] = {
    LED_DT_SPEC_GET(DT_NODELABEL(host_1_led)),
    LED_DT_SPEC_GET(DT_NODELABEL(host_2_led)),
    LED_DT_SPEC_GET(DT_NODELABEL(host_3_led)),
};

static bool blink_on;

static void update_work_handler(struct k_work *work) {
    int active = zmk_ble_active_profile_index();
    bool sleeping = zmk_activity_get_state() == ZMK_ACTIVITY_SLEEP;
    bool connected = active >= 0 && active < ARRAY_SIZE(profile_leds) &&
                     zmk_ble_profile_is_connected(active);

    blink_on = !blink_on;

    for (int i = 0; i < ARRAY_SIZE(profile_leds); i++) {
        uint8_t brightness = !sleeping && i == active && (connected || blink_on) ? 100 : 0;
        led_set_brightness_dt(&profile_leds[i], brightness);
    }

    if (!sleeping && !connected) {
        k_work_reschedule(k_work_delayable_from_work(work), K_MSEC(400));
    }
}

static K_WORK_DELAYABLE_DEFINE(update_work, update_work_handler);

static int event_listener(const zmk_event_t *eh) {
    blink_on = false;
    k_work_reschedule(&update_work, K_NO_WAIT);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(herox_host_status, event_listener);
ZMK_SUBSCRIPTION(herox_host_status, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(herox_host_status, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(herox_host_status, zmk_activity_state_changed);

static int init(void) {
    for (int i = 0; i < ARRAY_SIZE(profile_leds); i++) {
        if (!device_is_ready(profile_leds[i].dev)) {
            return -ENODEV;
        }
    }

    k_work_reschedule(&update_work, K_MSEC(100));
    return 0;
}

SYS_INIT(init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
