#ifndef COLORFORMAT_H
#define COLORFORMAT_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MATTER_COLOR_MAX 65279.0
#define MATTER_BRIGHTNESS_MAX 254.0


typedef struct {
    float red;
    float green;
    float blue;
} RGB_color_t;

typedef struct {
    long x;
    long y;
} XY_color_t;

typedef struct : public RGB_color_t {
    float white;
    float warmwhite;
} RGB_CCT_Duty_t;

//void RGB_to_RGBCCT(const RGB_color_t *rgb, RGB_CCT_Duty_t *rgbcct) {
//    rgbcct->red = rgb->red;
//    rgbcct->green = rgb->green;
//    rgbcct->blue = rgb->blue;
//    rgbcct->white = 0.0f;
//    rgbcct->warmwhite = 0.0f;
//}
//
//void RGBCCT_to_RGB(const RGB_CCT_Duty_t *rgbcct, RGB_color_t *rgb) {
//    rgb->red = rgbcct->red;
//    rgb->green = rgbcct->green;
//    rgb->blue = rgbcct->blue;
//}

void xy_to_duty(uint16_t cx, uint16_t cy, float brightness, RGB_color_t *RGB);

void scale_RGB_duty(float scale, RGB_color_t *RGB);

void colorTemperatureToRGB(uint32_t kelvin, RGB_color_t *RGB);

#ifdef __cplusplus
}
#endif

#endif // COLORFORMAT_H