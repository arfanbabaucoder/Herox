#define DT_DRV_COMPAT herox_behavior_feature

#include <errno.h>
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <dt-bindings/herox/feature.h>
#include <herox_x1/features.h>
#include <zmk/behavior.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    if (binding->param1 <= NX_RGB_SPEED_DOWN) {
        return herox_rgb_command(binding->param1);
    }

    if (binding->param1 <= NX_HAPTIC_TEST) {
        return herox_haptic_command(binding->param1);
    }

    return -ENOTSUP;
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api api = {
    .binding_pressed = on_pressed,
    .binding_released = on_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);

#endif
