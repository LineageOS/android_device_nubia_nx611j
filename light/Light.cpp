/*
 * Copyright (C) 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "LightService"

#include <log/log.h>

#include "Light.h"

#include <fstream>

#define LCD_LED         "/sys/class/leds/lcd-backlight/brightness"

#define NUBIA_LED_MODE    "/sys/class/leds/nubia_led/blink_mode"
#define NUBIA_LED_COLOR        "/sys/class/leds/nubia_led/outn"
#define NUBIA_GRADE    "/sys/class/leds/nubia_led/grade_parameter"
#define NUBIA_FADE     "/sys/class/leds/nubia_led/fade_parameter"

#define BATTERY_STATUS_FILE       "/sys/class/power_supply/battery/status"
#define BATTERY_CAPACITY          "/sys/class/power_supply/battery/capacity"

#define BATTERY_STATUS_DISCHARGING  "Discharging"
#define BATTERY_STATUS_NOT_CHARGING "Not charging"
#define BATTERY_STATUS_CHARGING     "Charging"

#define BLINK_MODE_ON    3
#define BLINK_MODE_CONST 1
#define BLINK_MODE_OFF   0

#define NUBIA_LED_DISABLE 16
#define NUBIA_LED_RED     48
#define NUBIA_LED_GREEN   64

#define BREATH_SOURCE_NOTIFICATION	0x01
#define BREATH_SOURCE_BATTERY		0x02
#define BREATH_SOURCE_BUTTONS		0x04
#define BREATH_SOURCE_ATTENTION		0x08
#define BREATH_SOURCE_NONE			0x00

#define MAX_LED_BRIGHTNESS    255
#define MAX_LCD_BRIGHTNESS    4095

static int32_t active_status = 0;

enum battery_status {
    BATTERY_UNKNOWN = 0,
    BATTERY_LOW,
    BATTERY_FREE,
    BATTERY_CHARGING,
    BATTERY_FULL,
};

namespace {
/*
 * Write value to path and close file.
 */
static void set(std::string path, std::string value) {
    std::ofstream file(path);

    if (!file.is_open()) {
        ALOGW("failed to write %s to %s", value.c_str(), path.c_str());
        return;
    }

    file << value;
}

static int get(std::string path) {
    std::ifstream file(path);
    int value;

    if (!file.is_open()) {
        ALOGW("failed to read from %s", path.c_str());
        return 0;
    }

    file >> value;
    return value;
}

static int readStr(std::string path, char *buffer, size_t size)
{

    std::ifstream file(path);

    if (!file.is_open()) {
        ALOGW("failed to read %s", path.c_str());
        return -1;
    }

    file.read(buffer, size);
    file.close();
    return 1;
}

static void set(std::string path, int value) {
    set(path, std::to_string(value));
}

static uint32_t getBrightness(const LightState& state) {
    uint32_t alpha, red, green, blue;

    /*
     * Extract brightness from AARRGGBB.
     */
    alpha = (state.color >> 24) & 0xFF;
    red = (state.color >> 16) & 0xFF;
    green = (state.color >> 8) & 0xFF;
    blue = state.color & 0xFF;

    /*
     * Scale RGB brightness if Alpha brightness is not 0xFF.
     */
    if (alpha != 0xFF) {
        red = red * alpha / 0xFF;
        green = green * alpha / 0xFF;
        blue = blue * alpha / 0xFF;
    }

    return (77 * red + 150 * green + 29 * blue) >> 8;
}

int getBatteryStatus()
{
    int err;

    char status_str[16];
    int capacity = 0;

    err = readStr(BATTERY_STATUS_FILE, status_str, sizeof(status_str));
    if (err <= 0) {
        ALOGI("failed to read battery status: %d", err);
        return BATTERY_UNKNOWN;
    }

    //ALOGI("battery status: %d, %s", err, status_str);

    capacity = get(BATTERY_CAPACITY);

    //ALOGI("battery capacity: %d", capacity);

    if (0 == strncmp(status_str, BATTERY_STATUS_CHARGING, 8)) {
        if (capacity < 90) {
            return BATTERY_CHARGING;
        } else {
            return BATTERY_FULL;
        }
    } else {
        if (capacity < 10) {
            return BATTERY_LOW;
        } else {
            return BATTERY_FREE;
        }
    }
}

static inline uint32_t scaleBrightness(uint32_t brightness, uint32_t maxBrightness) {
    return brightness * maxBrightness / 0xFF;
}

static inline uint32_t getScaledBrightness(const LightState& state, uint32_t maxBrightness) {
    return scaleBrightness(getBrightness(state), maxBrightness);
}

static void handleBacklight(const LightState& state) {
    uint32_t brightness = getScaledBrightness(state, MAX_LCD_BRIGHTNESS);
    set(LCD_LED, brightness);
}

static uint32_t setBreathLightLocked(uint32_t event_source, const LightState& state){
    uint32_t brightness = getScaledBrightness(state, MAX_LED_BRIGHTNESS);

    if (brightness > 0) {
        active_status |= event_source;
    } else {
        active_status &= ~event_source;
    }

    if(active_status == 0) { //nothing, close all
        // disable green led
        set(NUBIA_LED_COLOR, NUBIA_LED_GREEN);
        set(NUBIA_LED_MODE, BLINK_MODE_OFF);
        // disable red led
        set(NUBIA_LED_COLOR, NUBIA_LED_RED);
        set(NUBIA_LED_MODE, BLINK_MODE_OFF);
        // set disable led
        set(NUBIA_LED_COLOR, NUBIA_LED_DISABLE);
        set(NUBIA_LED_MODE, BLINK_MODE_OFF);
        set(NUBIA_FADE, "0 0 0");
        set(NUBIA_GRADE, "100 255");
        return 0;
    }

    if(active_status & BREATH_SOURCE_BATTERY) { //battery status
	    int battery_state = getBatteryStatus();
	    if(battery_state == BATTERY_CHARGING || battery_state == BATTERY_LOW){
            set(NUBIA_LED_COLOR, NUBIA_LED_RED);
            set(NUBIA_FADE, "0 0 0");
            set(NUBIA_GRADE, "100 255");
            set(NUBIA_LED_MODE, BLINK_MODE_CONST);
        }else if (battery_state == BATTERY_FULL){
            set(NUBIA_LED_COLOR, NUBIA_LED_GREEN);
            set(NUBIA_FADE, "0 0 0");
            set(NUBIA_GRADE, "100 255");
            set(NUBIA_LED_MODE, BLINK_MODE_CONST);
        }

        return 0;
    }

    if( (active_status & BREATH_SOURCE_NOTIFICATION ) || (active_status & BREATH_SOURCE_ATTENTION)) { //notification, set home breath
        int32_t onMS = state.flashOnMs;
        int32_t offMS = state.flashOffMs;
        switch(onMS){
        case 5000:
            onMS = 5;
            break;
        case 2000:
            onMS = 4;
            break;
        case 1000:
            onMS = 3;
            break;
        case 500:
            onMS = 2;
            break;
        case 250:
            onMS = 1;
            break;
        default:
            onMS = 1;
        }

        switch(offMS){
        case 5000:
            offMS = 5;
            break;
        case 2000:
            offMS = 4;
            break;
        case 1000:
            offMS = 3;
            break;
        case 500:
            offMS = 2;
            break;
        case 250:
            offMS = 1;
            break;
        case 1:
            offMS = 0;
            break;
        default:
            offMS = 0;
        }
        std::string fade_params = std::to_string(offMS) + " " + std::to_string(onMS) + " " + std::to_string(onMS);
        set(NUBIA_LED_COLOR, NUBIA_LED_GREEN);
        set(NUBIA_FADE, fade_params);
        set(NUBIA_GRADE, "0 100");
        set(NUBIA_LED_MODE, BLINK_MODE_ON);
    }
    return 0;
}

static inline bool isLit(const LightState& state) {
    return state.color & 0x00ffffff;
}

static void handleNotification(const LightState& state) {
    setBreathLightLocked(BREATH_SOURCE_NOTIFICATION, state);
}

static void handleBattery(const LightState& state){
    setBreathLightLocked(BREATH_SOURCE_BATTERY, state);
}

/* Keep sorted in the order of importance. */
static std::vector<LightBackend> backends = {
    { Type::ATTENTION, handleNotification },
    { Type::NOTIFICATIONS, handleNotification },
    { Type::BATTERY, handleBattery },
    { Type::BACKLIGHT, handleBacklight },
};

}  // anonymous namespace

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

Return<Status> Light::setLight(Type type, const LightState& state) {
    LightStateHandler handler;
    bool handled = false;

    /* Lock global mutex until light state is updated. */
    std::lock_guard<std::mutex> lock(globalLock);

    /* Update the cached state value for the current type. */
    for (LightBackend& backend : backends) {
        if (backend.type == type) {
            backend.state = state;
            handler = backend.handler;
        }
    }

    /* If no handler has been found, then the type is not supported. */
    if (!handler) {
        return Status::LIGHT_NOT_SUPPORTED;
    }

    /* Light up the type with the highest priority that matches the current handler. */
    for (LightBackend& backend : backends) {
        if (handler == backend.handler && isLit(backend.state)) {
            handler(backend.state);
            handled = true;
            break;
        }
    }

    /* If no type has been lit up, then turn off the hardware. */
    if (!handled) {
        handler(state);
    }

    return Status::SUCCESS;
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb _hidl_cb) {
    std::vector<Type> types;

    for (const LightBackend& backend : backends) {
        types.push_back(backend.type);
    }

    _hidl_cb(types);

    return Void();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android
