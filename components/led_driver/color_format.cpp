#include <color_format.h>

#include <stdlib.h>
#include <math.h>
#include <helpers.hpp>

/* Convert ColorXY to a 3 point float value representing the relative duty cycle for that color. */
void xy_to_duty(uint16_t cx, uint16_t cy, RGB_color_t *RGB)
{
    float x = cx / MATTER_COLOR_MAX;
    float y = cy / MATTER_COLOR_MAX;

    float X = x * (80. / y);
    float Y = 80.;
    float Z = (1 - x - y) * (80. / y);

    float r = X * 3.2404542 + Y * -1.5371385 + Z * -0.4985314;
    float g = X * -0.9692660 + Y * 1.8760108 + Z * 0.0415560;
    float b = X * 0.0556434 + Y * -0.2040259 + Z * 1.0572252;

    float min = std::min(std::min(r, g), b);
    if (min < 0)
    {
        r -= min;
        g -= min;
        b -= min;
    }

    r = ((r > 0.0031308) ? (1.055 * pow(r, 1 / 2.4) - 0.055) : (12.92 * r));
    g = ((g > 0.0031308) ? (1.055 * pow(g, 1 / 2.4) - 0.055) : (12.92 * g));
    b = ((b > 0.0031308) ? (1.055 * pow(b, 1 / 2.4) - 0.055) : (12.92 * b));

    float max = std::max(std::max(r, g), b);

    RGB->red   =  clamp<float>(r/max, 0, 1);
    RGB->green =  clamp<float>(g/max, 0, 1);
    RGB->blue  =  clamp<float>(b/max, 0, 1);
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