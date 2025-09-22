#include <color_format.h>

#include <stdlib.h>
#include <math.h>
#include <helpers.hpp>

/* Convert ColorXY to a 3 point float value representing the relative duty cycle for that color. */
// xy to RGB with brightness scaling
void xy_to_duty(uint16_t cx, uint16_t cy, float brightness, RGB_color_t *RGB)
{
    // 1. Normalize input coordinates from Matter's uint16_t to float [0, 1]
    // The spec uses 0-65535, so we divide by 65535.0
    float x = static_cast<float>(cx) / 65535.0f;
    float y = static_cast<float>(cy) / 65535.0f;

    // A basic check for invalid coordinates
    if (y <= 0.0f || (x + y) > 1.0f) {
        RGB->red = 0.0f;
        RGB->green = 0.0f;
        RGB->blue = 0.0f;
        return;
    }

    // 2. Use the brightness parameter for Luminance (Y)
    // The brightness parameter should be in the range [0.0, 1.0]
    float Y = brightness;

    // 3. Convert from xyY to XYZ color space
    float X = (x * Y) / y;
    float Z = ((1.0f - x - y) * Y) / y;

    // 4. Convert from XYZ to linear sRGB
    // This uses the standard D65 illuminant matrix for sRGB.
    float r_linear = X * 3.2404542f + Y * -1.5371385f + Z * -0.4985314f;
    float g_linear = X * -0.9692660f + Y * 1.8760108f + Z * 0.0415560f;
    float b_linear = X * 0.0556434f + Y * -0.2040259f + Z * 1.0572252f;

    // 5. Clip linear values to handle out-of-gamut colors properly.
    // First, simply clip any negative values to 0. This is better than adding white.
    r_linear = std::max(0.0f, r_linear);
    g_linear = std::max(0.0f, g_linear);
    b_linear = std::max(0.0f, b_linear);

    // Second, if any component is > 1.0, the color is too bright to be displayed.
    // Scale all components down to fit within the gamut while preserving the hue.
    float max_comp = std::max({r_linear, g_linear, b_linear});
    if (max_comp > 1.0f) {
        r_linear /= max_comp;
        g_linear /= max_comp;
        b_linear /= max_comp;
    }

    // 6. Apply gamma correction to get non-linear sRGB values for display
    auto gamma_correct = [](float c) {
        return (c > 0.0031308f) ? (1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f)
                                 : (12.92f * c);
    };

    // 7. Store the final, clamped values
    RGB->red   = std::clamp(gamma_correct(r_linear), 0.0f, 1.0f);
    RGB->green = std::clamp(gamma_correct(g_linear), 0.0f, 1.0f);
    RGB->blue  = std::clamp(gamma_correct(b_linear), 0.0f, 1.0f);
}
/* --------------------------------------------------------------------------------------------- */


/* Scale all RGB values by the passed amount */
void scale_RGB_duty(float scale, RGB_color_t *RGB){
    RGB->red   = clamp<float>((RGB->red   * scale), 0.f, 1.f);
    RGB->green = clamp<float>((RGB->green * scale), 0.f, 1.f);
    RGB->blue  = clamp<float>((RGB->blue  * scale), 0.f, 1.f);
}
/* ----------------------------------------- */

/* Estimates the RGB equivilant of the passed color data */
void colorTemperatureToRGB(uint32_t kelvin, RGB_color_t *RGB){

    float temp = clamp<uint32_t>(kelvin, 1000, 40000) / 100;

    float r, g, b;

    if( temp <= 66 ){ 

        r = 255;
        
        g = temp;
        g = 99.4708025861 * log(g) - 161.1195681661;
        g = clamp<float>(g, 0, 255);
        
        if( temp <= 19){
            b = 0;
        } else {
            b = temp - 10;
            b = 138.5177312231 * log(b) - 305.0447927307;
            b = clamp<float>(b, 0, 255);
        }

    } else {

        r = temp - 60;
        r = 329.698727446 * pow(r, -0.1332047592);
        
        g = temp - 60;
        g = 288.1221695283 * pow(g, -0.0755148492 );

        b = 255;
    }

    RGB->red   =  clamp<float>(r/255, 0, 1);
    RGB->green =  clamp<float>(g/255, 0, 1);
    RGB->blue  =  clamp<float>(b/255, 0, 1);
}
/* ----------------------------------------------------- */