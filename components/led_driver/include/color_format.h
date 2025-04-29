#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MATTER_COLOR_MAX 65279.0
#define MATTER_BRIGHTNESS_MAX 254.0

typedef struct {
    long x;
    long y;
} XY_color_t;

typedef struct {
    float red;
    float green;
    float blue;
    float white;
    float warmwhite;
} RGB_CCT_Duty_t;

void xy_to_duty(uint16_t cx, uint16_t cy, RGB_color_t *RGB);

void scale_RGB_duty(float scale, RGB_color_t *RGB);

void colorTemperatureToRGB(uint32_t kelvin, RGB_color_t *RGB);

#ifdef __cplusplus
}
#endif
