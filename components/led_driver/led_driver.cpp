#include <esp_log.h>
#include <led_driver.h>
#include <math.h>
#include <helpers.hpp>
#include <inttypes.h> 

/* Generates the channel configs, then initialize them */
esp_err_t LED_Driver::enable_LEDC_Channel(led_channel_info_t config) {
    // Channel already enabled or missing gpio value then skip
    if (channel_enabled[config.channel] || config.gpio == -1){
        return ESP_OK;
    }

    ledc_channel_config_t ledc_channel_cfg =
        {
            .gpio_num   = config.gpio,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel    = config.channel,
            .intr_type  = LEDC_INTR_FADE_END,
            .timer_sel  = timer,
            .duty       = 0,
            .hpoint     = 0,
        };

    channel_enabled[config.channel] = true;
    return ledc_channel_config(&ledc_channel_cfg);
}
esp_err_t LED_Driver::disable_LEDC_Channel(led_channel_info_t config) {
    // Channel already disabled, skip
    if (channel_enabled[config.channel] == false){
        return ESP_OK;
    }

    channel_enabled[config.channel] = false;

    // Stop PWM and hold the pin low
    esp_err_t err = ledc_stop(LEDC_SPEED_MODE, config.channel, 0);
    err |= gpio_set_level((gpio_num_t)config.gpio, 0); // Ensure pin is held low

    return err;
}

/* Initialize the led driver and the shared instance struct */
LED_Driver::LED_Driver(LED_GPIO_MAP pins_) {
    ESP_LOGW(TAG, "Initializing light driver");

    max_pwm = pow(2, 13);

    // Apply configuration to timer
    ledc_timer_config(&ledc_timer);

    
    /* Generate Configurations for the needed channels */
    enable_LEDC_Channel(pins_.red);
    enable_LEDC_Channel(pins_.green);
    enable_LEDC_Channel(pins_.blue);
    enable_LEDC_Channel(pins_.white);
    enable_LEDC_Channel(pins_.warmwhite);

    pins = pins_;

    ledc_fade_func_install(0);
}
/* -------------------------------------------------------- */


/* Sets the duty cycle to zero, effectively turning off the LEDs */
esp_err_t LED_Driver::set_power(bool new_power){
    ESP_LOGW(TAG, "Setting power to: %d", power);
    power = new_power;

    if (power) {
        bri = (obri == 0) ? DEFAULT_BRIGHTNESS : bri; // If brightness was 0 set to default
    } 
    else {
        obri = bri; // Save the old brightness for when we turn back on
        bri = 0;    // Set brightness to 0
    }

    return set_brightness(bri);
}
/* -------------------------------------------------------- */

/* Converts the internal duty cycle format to the needed PWM value */
uint32_t LED_Driver::duty_to_pwm(float color) {
    // Use integer math for efficiency on embedded systems
    uint32_t value = (uint32_t)((color * bri * max_pwm) / 100.0f);

    ESP_LOGW(TAG, "Color: %f, Brightness: %d, PWM: %" PRIu32, color, bri, value);
    return value;
}
/* ---------------------------------------------------------------- */

esp_err_t LED_Driver::set_channel_duty(float *new_color, float *old_color, led_channel_info_t channelConfig){
    // If the channel isn't enabled, or if the color/brightness hasn't changed skip
    if(new_color == old_color){
        // Nothing changed on this channel skip
        return ESP_OK;
    }

    // Update Tracker Used To Speed Up Future Checks
    old_color = new_color;

    // Disable channel if we are trying to set it to 0
    if (new_color == 0 && channel_enabled[channelConfig.channel] == true){
        disable_LEDC_Channel(channelConfig);
        return ESP_OK;
    } 

    // Enable channel if it was off and we are trying to set a value
    if (new_color != 0 && channel_enabled[channelConfig.channel] == false){
        enable_LEDC_Channel(channelConfig);
    }
    
    return ledc_set_duty_and_update(LEDC_SPEED_MODE, channelConfig.channel, duty_to_pwm(*new_color), 0);
}

/* Sets the duty cycle for each LEDC Channel */
esp_err_t LED_Driver::set_duty(){
    ESP_LOGW(TAG, "Setting RGB to: %f, %f, %f, %f, %f", nRGB.red, nRGB.green, nRGB.blue, nRGB.white, nRGB.warmwhite);
    esp_err_t err = ESP_OK;

    err |= set_channel_duty(&nRGB.red, &RGB.red, pins.red);
    err |= set_channel_duty(&nRGB.green, &RGB.green, pins.green);
    err |= set_channel_duty(&nRGB.blue, &RGB.blue, pins.blue);
    err |= set_channel_duty(&nRGB.white, &RGB.white, pins.white);
    err |= set_channel_duty(&nRGB.warmwhite, &RGB.warmwhite, pins.warmwhite);
    
    return err;
}
/* ----------------------------------------- */



/* Sets the brightness for each channel */
esp_err_t LED_Driver::set_brightness(uint8_t brightness){
    ESP_LOGW(TAG, "Setting brightness to: %d", brightness);

    if (!power) {
        bri = 0;
    } else {
        bri = brightness;
    }

    return set_duty();
}
/* ---------------------------------------- */

/* Determines the needed channel duty cycles for a given temperature  */
esp_err_t LED_Driver::set_temperature(uint32_t temperature){
    ESP_LOGW(TAG, "Setting temperature to: %lu", temperature);

    float white = clamp<float>(temperature/10000.0, 0, 1);

    nRGB.white = white;
    nRGB.warmwhite = 1.0-white;

    nRGB.red   = 0;
    nRGB.green = 0;
    nRGB.blue  = 0;

    return set_duty();
}
/* -----------------------------------------------------------------  */

/* Determines the needed channel duty cycles for a given ColorXY value */
esp_err_t LED_Driver::set_colorXY(long x, long y){
    ESP_LOGW(TAG, "Setting ColorXY to: (%lu, %lu)", x, y);

    if(x != -1){
        XY.x = x;
    }
    if(y != -1){
        XY.y = y;
    }

    if(XY.x != -1 && XY.y != -1) {
        nRGB.white = 0;
        nRGB.warmwhite = 0;

        xy_to_duty(XY.x, XY.y, bri, &nRGB);
        XY = {-1, -1};
        return set_duty();
    }

    return ESP_OK;
}
/* ----------------------------------------------------------------- */