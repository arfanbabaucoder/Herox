#include <errno.h>
#include <stdlib.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/spinlock.h>
#include <zephyr/sys/util.h>
#include <dt-bindings/herox/feature.h>
#include <herox_x1/features.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>

#define STRIP_NODE DT_NODELABEL(key_leds)
#define PIXEL_COUNT DT_PROP(STRIP_NODE, chain_length)
#define EFFECT_COUNT 8

BUILD_ASSERT(PIXEL_COUNT == 104);

struct rgb_state {
    uint8_t version;
    uint8_t enabled;
    uint8_t effect;
    uint16_t hue;
    uint8_t saturation;
    uint8_t brightness;
    uint8_t speed;
};

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb pixels[PIXEL_COUNT];
static struct rgb_state state = {1, 1, 3, 205, 100, 65, 3};
static uint8_t energy[PIXEL_COUNT];
static uint32_t animation_step;
static bool strip_was_on;
static struct k_spinlock energy_lock;

static const uint16_t key_x[PIXEL_COUNT] = {
    0, 200, 300, 400, 500, 650, 750, 850, 950, 1100, 1200, 1300, 1400,
    1525, 1625, 1725, 0, 100, 200, 300, 400, 500, 600, 700, 800, 900,
    1000, 1100, 1200, 1350, 1525, 1625, 1725, 1850, 1950, 2050, 2150,
    25, 150, 250, 350, 450, 550, 650, 750, 850, 950, 1050, 1150, 1250,
    1375, 1525, 1625, 1725, 1850, 1950, 2050, 2150, 38, 175, 275, 375,
    475, 575, 675, 775, 875, 975, 1075, 1175, 1338, 1850, 1950, 2050,
    63, 225, 325, 425, 525, 625, 725, 825, 925, 1025, 1125, 1313, 1625,
    1850, 1950, 2050, 2150, 13, 138, 263, 638, 1013, 1138, 1263, 1388,
    1525, 1625, 1725, 1900, 2050,
};

static const uint16_t key_y[PIXEL_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
    150, 150, 150, 150, 150, 150, 150,
    250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250,
    250, 250, 250, 250, 250, 250, 300,
    350, 350, 350, 350, 350, 350, 350, 350, 350, 350, 350, 350, 350, 350,
    350, 350,
    450, 450, 450, 450, 450, 450, 450, 450, 450, 450, 450, 450, 450, 450,
    450, 450, 500,
    550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550,
};

static struct led_rgb hsv(uint16_t hue, uint8_t saturation, uint8_t value) {
    uint16_t h = hue % 360;
    uint8_t region = h / 60;
    uint16_t remainder = (h % 60) * 255 / 60;
    uint8_t p = (uint16_t)value * (255 - saturation) / 255;
    uint8_t q = (uint16_t)value * (255 - ((uint16_t)saturation * remainder / 255)) / 255;
    uint8_t t = (uint16_t)value * (255 - ((uint16_t)saturation * (255 - remainder) / 255)) / 255;

    switch (region) {
    case 0:
        return (struct led_rgb){value, t, p};
    case 1:
        return (struct led_rgb){q, value, p};
    case 2:
        return (struct led_rgb){p, value, t};
    case 3:
        return (struct led_rgb){p, q, value};
    case 4:
        return (struct led_rgb){t, p, value};
    default:
        return (struct led_rgb){value, p, q};
    }
}

static uint8_t triangle(uint16_t phase) {
    phase %= 510;
    return phase <= 255 ? phase : 510 - phase;
}

static uint32_t hash32(uint32_t value) {
    value ^= value >> 16;
    value *= 0x7feb352d;
    value ^= value >> 15;
    value *= 0x846ca68b;
    return value ^ (value >> 16);
}

static void render_solid(void) {
    struct led_rgb color = hsv(state.hue, state.saturation, state.brightness * 255 / 100);
    for (int i = 0; i < PIXEL_COUNT; i++) {
        pixels[i] = color;
    }
}

static void render_breathe(void) {
    uint8_t pulse = triangle(animation_step * state.speed);
    uint8_t value = (uint16_t)state.brightness * pulse * 255 / 100 / 255;
    struct led_rgb color = hsv(state.hue, state.saturation, value);
    for (int i = 0; i < PIXEL_COUNT; i++) {
        pixels[i] = color;
    }
}

static void render_spectrum(void) {
    for (int i = 0; i < PIXEL_COUNT; i++) {
        pixels[i] = hsv(animation_step * state.speed / 2 + i * 360 / PIXEL_COUNT, 255,
                        state.brightness * 255 / 100);
    }
}

static void render_wave(void) {
    for (int i = 0; i < PIXEL_COUNT; i++) {
        uint16_t hue = state.hue + key_x[i] * 180 / 2150 + animation_step * state.speed;
        uint8_t wave = 120 + triangle(key_x[i] / 5 + animation_step * state.speed * 3) / 2;
        uint8_t value = (uint16_t)state.brightness * wave / 100;
        pixels[i] = hsv(hue, state.saturation, MIN(value, 255));
    }
}

static void render_reactive(void) {
    uint8_t snapshot[PIXEL_COUNT];
    k_spinlock_key_t key = k_spin_lock(&energy_lock);
    memcpy(snapshot, energy, sizeof(snapshot));
    for (int i = 0; i < PIXEL_COUNT; i++) {
        energy[i] = energy[i] > 7 ? energy[i] - 7 : 0;
    }
    k_spin_unlock(&energy_lock, key);

    for (int i = 0; i < PIXEL_COUNT; i++) {
        uint8_t peak = 0;
        for (int source = 0; source < PIXEL_COUNT; source++) {
            if (!snapshot[source]) {
                continue;
            }
            uint16_t distance = abs((int)key_x[i] - key_x[source]) +
                                abs((int)key_y[i] - key_y[source]);
            uint16_t radius = (255 - snapshot[source]) * (7 + state.speed) / 2;
            uint16_t delta = abs((int)distance - radius);
            uint8_t intensity = delta < 260 ? 255 - delta * 255 / 260 : 0;
            peak = MAX(peak, intensity);
        }
        uint8_t value = 3 + (uint16_t)state.brightness * peak * 252 / 100 / 255;
        pixels[i] = hsv(state.hue + key_x[i] / 10, state.saturation, value);
    }
}

static void render_heatmap(void) {
    k_spinlock_key_t key = k_spin_lock(&energy_lock);
    for (int i = 0; i < PIXEL_COUNT; i++) {
        uint8_t heat = energy[i];
        energy[i] = heat > 2 ? heat - 2 : 0;
        uint16_t hue = 240 - (uint16_t)heat * 240 / 255;
        uint8_t value = 5 + (uint16_t)state.brightness * heat * 250 / 100 / 255;
        pixels[i] = hsv(hue, 255, value);
    }
    k_spin_unlock(&energy_lock, key);
}

static void render_starlight(void) {
    uint32_t frame = animation_step * state.speed / 12;
    for (int i = 0; i < PIXEL_COUNT; i++) {
        uint32_t seed = hash32(i * 971U + frame / 48U * 7919U);
        uint8_t phase = frame + (seed & 0xff);
        uint8_t sparkle = (seed % 13 == 0) ? triangle(phase * 5) : 5;
        uint8_t value = (uint16_t)state.brightness * sparkle / 100;
        pixels[i] = hsv(state.hue + (seed % 70), state.saturation / 2, value);
    }
}

static void render_aurora(void) {
    for (int i = 0; i < PIXEL_COUNT; i++) {
        uint8_t band = triangle(key_x[i] / 7 + key_y[i] / 5 + animation_step * state.speed * 2);
        uint16_t hue = state.hue + 70 + band * 100 / 255;
        uint8_t value = (uint16_t)state.brightness * (100 + band * 155 / 255) / 100;
        pixels[i] = hsv(hue, 210, MIN(value, 255));
    }
}

static void apply_current_limit(void) {
    uint32_t channel_sum = 0;
    for (int i = 0; i < PIXEL_COUNT; i++) {
        channel_sum += pixels[i].r + pixels[i].g + pixels[i].b;
    }

    uint32_t limit = CONFIG_HEROX_X1_RGB_CURRENT_LIMIT_MA * 255U / 20U;
    if (channel_sum <= limit || !channel_sum) {
        return;
    }

    for (int i = 0; i < PIXEL_COUNT; i++) {
        pixels[i].r = (uint32_t)pixels[i].r * limit / channel_sum;
        pixels[i].g = (uint32_t)pixels[i].g * limit / channel_sum;
        pixels[i].b = (uint32_t)pixels[i].b * limit / channel_sum;
    }
}

static void render_work_handler(struct k_work *work) {
    if (!device_is_ready(strip)) {
        return;
    }

    if (!state.enabled || !zmk_usb_is_powered()) {
        if (strip_was_on) {
            memset(pixels, 0, sizeof(pixels));
            led_strip_update_rgb(strip, pixels, PIXEL_COUNT);
            strip_was_on = false;
        }
        return;
    }

    switch (state.effect) {
    case 0:
        render_solid();
        break;
    case 1:
        render_breathe();
        break;
    case 2:
        render_spectrum();
        break;
    case 3:
        render_wave();
        break;
    case 4:
        render_reactive();
        break;
    case 5:
        render_heatmap();
        break;
    case 6:
        render_starlight();
        break;
    default:
        render_aurora();
        break;
    }

    apply_current_limit();
    led_strip_update_rgb(strip, pixels, PIXEL_COUNT);
    strip_was_on = true;
    animation_step++;
    k_work_reschedule(k_work_delayable_from_work(work),
                      K_MSEC(CONFIG_HEROX_X1_RGB_FRAME_MS));
}

static K_WORK_DELAYABLE_DEFINE(render_work, render_work_handler);

static void save_work_handler(struct k_work *work) {
    settings_save_one("herox/rgb/state", &state, sizeof(state));
}

static K_WORK_DELAYABLE_DEFINE(save_work, save_work_handler);

static int settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    const char *next;
    struct rgb_state loaded;

    if (!settings_name_steq(name, "state", &next) || next || len != sizeof(loaded)) {
        return -ENOENT;
    }

    int rc = read_cb(cb_arg, &loaded, sizeof(loaded));
    if (rc < 0) {
        return rc;
    }

    if (loaded.version == 1 && loaded.effect < EFFECT_COUNT && loaded.hue < 360 &&
        loaded.saturation <= 100 && loaded.brightness <= 100 && loaded.speed >= 1 &&
        loaded.speed <= 5) {
        state = loaded;
    }

    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(herox_rgb, "herox/rgb", NULL, settings_set, NULL, NULL);

static void changed(void) {
    animation_step = 0;
    k_work_reschedule(&save_work, K_SECONDS(1));
    k_work_reschedule(&render_work, K_NO_WAIT);
}

int herox_rgb_command(uint32_t command) {
    switch (command) {
    case NX_RGB_TOGGLE:
        state.enabled = !state.enabled;
        break;
    case NX_RGB_EFFECT_NEXT:
        state.effect = (state.effect + 1) % EFFECT_COUNT;
        break;
    case NX_RGB_EFFECT_PREVIOUS:
        state.effect = (state.effect + EFFECT_COUNT - 1) % EFFECT_COUNT;
        break;
    case NX_RGB_HUE_UP:
        state.hue = (state.hue + 12) % 360;
        break;
    case NX_RGB_HUE_DOWN:
        state.hue = (state.hue + 348) % 360;
        break;
    case NX_RGB_BRIGHTNESS_UP:
        state.brightness = MIN(state.brightness + 5, 100);
        break;
    case NX_RGB_BRIGHTNESS_DOWN:
        state.brightness = state.brightness >= 5 ? state.brightness - 5 : 0;
        break;
    case NX_RGB_SPEED_UP:
        state.speed = MIN(state.speed + 1, 5);
        break;
    case NX_RGB_SPEED_DOWN:
        state.speed = state.speed > 1 ? state.speed - 1 : 1;
        break;
    default:
        return -ENOTSUP;
    }

    changed();
    return 0;
}

static int event_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *position = as_zmk_position_state_changed(eh);

    if (position && position->state && position->position < PIXEL_COUNT) {
        k_spinlock_key_t key = k_spin_lock(&energy_lock);
        energy[position->position] = 255;
        k_spin_unlock(&energy_lock, key);
        if (state.effect == 4 || state.effect == 5) {
            k_work_reschedule(&render_work, K_NO_WAIT);
        }
    }

    if (as_zmk_usb_conn_state_changed(eh)) {
        k_work_reschedule(&render_work, K_NO_WAIT);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(herox_rgb, event_listener);
ZMK_SUBSCRIPTION(herox_rgb, zmk_position_state_changed);
ZMK_SUBSCRIPTION(herox_rgb, zmk_usb_conn_state_changed);

static int init(void) {
    if (!device_is_ready(strip)) {
        return -ENODEV;
    }

    k_work_reschedule(&render_work, K_MSEC(100));
    return 0;
}

SYS_INIT(init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
