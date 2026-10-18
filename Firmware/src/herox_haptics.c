#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/haptics.h>
#include <zephyr/drivers/haptics/drv2605.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/atomic.h>
#include <dt-bindings/herox/feature.h>
#include <herox_x1/features.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

#define HAPTIC_NODE DT_NODELABEL(haptic)

static const struct device *const haptic = DEVICE_DT_GET(HAPTIC_NODE);
static atomic_t pending_effect;
static uint8_t level = CONFIG_HEROX_X1_HAPTIC_DEFAULT_LEVEL;

static void save_work_handler(struct k_work *work) {
    settings_save_one("herox/haptic/level", &level, sizeof(level));
}

static K_WORK_DELAYABLE_DEFINE(save_work, save_work_handler);

static void playback_work_handler(struct k_work *work) {
    uint8_t effect = atomic_set(&pending_effect, 0);

    if (!effect || !device_is_ready(haptic)) {
        return;
    }

    struct drv2605_rom_data rom = {
        .trigger = DRV2605_MODE_INTERNAL_TRIGGER,
        .library = DRV2605_LIBRARY_LRA,
        .seq_regs = {effect, 0, 0, 0, 0, 0, 0, 0},
    };
    union drv2605_config_data config = {.rom_data = &rom};

    if (drv2605_haptic_config(haptic, DRV2605_HAPTICS_SOURCE_ROM, &config) == 0) {
        haptics_start_output(haptic);
    }
}

static K_WORK_DEFINE(playback_work, playback_work_handler);

static void play(uint8_t effect) {
    if (!level) {
        return;
    }

    atomic_set(&pending_effect, effect);
    k_work_submit(&playback_work);
}

static int settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    const char *next;

    if (!settings_name_steq(name, "level", &next) || next || len != sizeof(level)) {
        return -ENOENT;
    }

    int rc = read_cb(cb_arg, &level, sizeof(level));
    if (rc >= 0) {
        level = MIN(level, 3);
        return 0;
    }

    return rc;
}

SETTINGS_STATIC_HANDLER_DEFINE(herox_haptic, "herox/haptic", NULL, settings_set, NULL, NULL);

static int event_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);

    if (!event || !event->state || !level) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    static const uint8_t effects[] = {0, 3, 2, 1};
    uint8_t effect = effects[level];

    if (event->position == 29 || event->position == 57 || event->position == 70 ||
        event->position == 74 || event->position == 85 || event->position == 90 ||
        event->position == 94) {
        effect = level == 1 ? 2 : 1;
    }

    play(effect);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(herox_haptics, event_listener);
ZMK_SUBSCRIPTION(herox_haptics, zmk_position_state_changed);

int herox_haptic_command(uint32_t command) {
    switch (command) {
    case NX_HAPTIC_TOGGLE:
        level = level ? 0 : CONFIG_HEROX_X1_HAPTIC_DEFAULT_LEVEL;
        break;
    case NX_HAPTIC_UP:
        level = MIN(level + 1, 3);
        break;
    case NX_HAPTIC_DOWN:
        level = level ? level - 1 : 0;
        break;
    case NX_HAPTIC_TEST:
        if (!level) {
            return 0;
        }
        play(level == 1 ? 3 : level == 2 ? 2 : 1);
        return 0;
    default:
        return -ENOTSUP;
    }

    k_work_reschedule(&save_work, K_SECONDS(1));
    play(level == 1 ? 3 : level == 2 ? 2 : 1);
    return 0;
}

static int init(void) {
    return device_is_ready(haptic) ? 0 : -ENODEV;
}

SYS_INIT(init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
