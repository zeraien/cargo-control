#include "read_inputs.h"

#include "Arduino.h"
#include "lib.h"
#include "settings.h"

SWITCH_STATUS read_inputs() {
    return {
        .blink_left = read(SW_BLINK_L) || read(K1),
        .blink_right = read(SW_BLINK_R) || read(K2),
        .horn = read(SW_HORN) || read(K4),
        .alert = read(SW_ALERT) || read(K3),
        .box_light = read(SW_BOX_LIGHT),
        .position_light = read(SW_POS),
        .four_way = read(SW_4WAY),
        .drl = true
    };
}