#include <color_format.h>
#include <esp_log.h>
#include <led_driver.h>
#include "driver/ledc.h"
#include <math.h>
#include <helpers.hpp>
#include <inttypes.h> 

/* Generates the channel configs, then initialize them */
esp_err_t LED_Driver::generateChannelConfig(uint8_t gpio, uint4_t channel) {
    ledc_channel_config_t ledc_channel_cfg =
        {
            .gpio_num   = gpio,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel    = channel,
            .intr_type  = LEDC_INTR_FADE_END,
            .timer_sel  = timer,
            .duty       = 0,
            .hpoint     = 0,
        }

        channel_enabled[channel] = true;
        return ledc_channel_config(&ledc_channel_cfg);
}

/* Initialize the led driver and the shared instance struct */
LED_Driver::LED_Driver(LED_GPIO_MAP gpio_pins) {
    ESP_LOGI(TAG, "Initializing light driver");

    max_pwm = pow(2, 13);

    // Apply configuration to timer
    ledc_timer_config(&ledc_timer);

    /* Generate Configurations for the needed channels */
    if (gpio_pins.red != -1){
        generateChannelConfig(gpio_pins.red, CHANNEL_RED, LEDC_TIMER_0);
    }
    if (gpio_pins.green != -1){
        generateChannelConfig(gpio_pins.green, CHANNEL_GREEN, LEDC_TIMER_0);
    }
    if (gpio_pins.blue != -1){
        generateChannelConfig(gpio_pins.blue, CHANNEL_BLUE, LEDC_TIMER_0);
    }
    if (gpio_pins.white != -1){
        generateChannelConfig(gpio_pins.white, CHANNEL_WHITE, LEDC_TIMER_0);
    }
    if (gpio_pins.warmwhite != -1){
        generateChannelConfig(gpio_pins.warmwhite, CHANNEL_WARMWHITE, LEDC_TIMER_0);
    }

    // Currently Unneeded but allows for the fade func
    ledc_fade_func_install(0);
}
/* -------------------------------------------------------- */

/* Sets the duty cycle to zero, effectively turning off the LEDs */
LED_Driver::set_power(bool new_power){
    ESP_LOGI(TAG, "Setting power to: %d", power);
    power = new_power;

    return set_brightness(power);
}
/* -------------------------------------------------------- */

/* Converts the internal duty cycle format to the needed PWM value */
LED_Driver::duty_to_pwm(float color) {
    color *= (brightness/100.f);
    uint32_t value = static_cast<uint32_t>(max_pwm*color);
    return value;
}
/* ---------------------------------------------------------------- */

esp_err_t LED_Driver::set_channel_duty(float *new_color, float *old_color, ledc_channel_t channel){
    // If the channel isn't enabled, or if the color/brightness hasn't changed skip
    if(!channel_enabled[channel] || ((new_color == old_color) && (nbri == bri))){
        // Nothing changed on this channel skip
        return ESP_OK;
    }
    // Duty changed so apply new value
    &old_color = &new_color;
    return ledc_set_duty_and_update(LEDC_SPEED_MODE, channel, duty_to_pwm(&new_color), 0);
}

/* Sets the duty cycle for each LEDC Channel */
LED_Driver::set_duty(){
    ESP_LOGI(TAG, "Setting RGB to: %f, %f, %f", nRGB.red, nRGB.green, nRGB.blue);
    esp_err_t err = ESP_OK;

    err |= set_channel_duty(nRGB.red, RGB.red, CHANNEL_RED);
    err |= set_channel_duty(nRGB.green, RGB.green, CHANNEL_GREEN);
    err |= set_channel_duty(nRGB.blue, RGB.blue, CHANNEL_BLUE);
    err |= set_channel_duty(nRGB.white, RGB.white, CHANNEL_WHITE);
    err |= set_channel_duty(nRGB.warmwhite, RGB.warmwhite, CHANNEL_WARMWHITE);
    bri = nbri;
    
    return err;
}
/* ----------------------------------------- */



/* Sets the brightness for each channel */
LED_Driver::set_brightness(uint8_t brightness){
    ESP_LOGI(TAG, "Setting brightness to: %d", brightness);

    if (!power) {
        bri = 0;
    } else {
        bri = brightness;
    }

    return set_duty();
}
/* ---------------------------------------- */

/* Determines the needed channel duty cycles for a given temperature  */
LED_Driver::set_temperature(uint32_t temperature){
    ESP_LOGI(TAG, "Setting temperature to: %lu", temperature);

    float white = clamp<float>(temperature/10000.0, 0, 1);

    nRGB.white = white;
    nRGB.warmwhite = 1.0-white;

    nRGB.red   = 0;
    nRGB.green = 0;
    nRGB.blue  = 0;


    return set_duty;
}
/* -----------------------------------------------------------------  */

/* Determines the needed channel duty cycles for a given ColorXY value */
LED_Driver::set_colorXY(long x, long y){
    ESP_LOGI(TAG, "Setting ColorXY to: (%lu, %lu)", x, y);
    
    if(x != -1){
        XY.x = x;
    }
    if(y != -1){
        XY.y = y;
    }

    if(XY.x != -1 && XY.y != -1) {
        nRGB.white = 0;
        nRGB.warmwhite = 0;

        xy_to_duty(XY.x, XY.y, &nRGB);
        XY = {-1, -1};
        return set_duty();
    }

    return ESP_OK;
}
/* ----------------------------------------------------------------- */