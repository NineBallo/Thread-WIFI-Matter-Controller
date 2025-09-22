#ifndef HELPERS_H
#define HELPERS_H

#include <algorithm>


template<typename T>
T clamp(T val, T mn, T mx){
return std::max(std::min(val, mx), mn);
}

template<typename T, uint8_t LEN>
void set_all_values_in_array(T (&arr)[LEN], T val) {
    for (T& value : arr) {
        value = val;
    }
}

/** Standard max values (used for remapping attributes) */
#define STANDARD_BRIGHTNESS 100
#define STANDARD_HUE 360
#define STANDARD_SATURATION 100
#define STANDARD_TEMPERATURE_FACTOR 1000000
#define STANDARD_COLOR 1

/** Matter max values (used for remapping attributes) */
#define MATTER_BRIGHTNESS 254
#define MATTER_HUE 254
#define MATTER_SATURATION 254
#define MATTER_TEMPERATURE_FACTOR 1000000
#define MATTER_COLOR_MAX 65279

/** Default attribute values used during initialization */
#define DEFAULT_POWER true
#define DEFAULT_BRIGHTNESS 64


/* Pin mappings from RGBWW to the respective GPIO values */
#define PIN_R  (22)
#define PIN_G  (3)
#define PIN_B  (21)
#define PIN_W  (1)
#define PIN_WW (0)
/* ----------------------------------------------------- */

#endif // HELPERS_H