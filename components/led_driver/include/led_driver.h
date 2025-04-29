#pragma once
#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Channel mappings from RGBWW to the respective channels */
#define CHANNEL_RED        LEDC_CHANNEL_0
#define CHANNEL_GREEN      LEDC_CHANNEL_1
#define CHANNEL_BLUE       LEDC_CHANNEL_2
#define CHANNEL_WHITE      LEDC_CHANNEL_3
#define CHANNEL_WARMWHITE  LEDC_CHANNEL_4
/* ----------------------------------------------------- */


/* Configures the channel and timer paramters */
#define ESP32C6_MAX_CHANNELS 6
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
/* ------------------------------------------ */

typedef struct {
    uint8_t red       = -1;
    uint8_t green     = -1;
    uint8_t blue      = -1;
    uint8_t white     = -1;
    uint8_t warmwhite = -1;
} LED_GPIO_MAP;

class LED_Driver {
    LED_Driver(LED_GPIO_MAP gpio_pins);
    ~LED_Driver();

    public:
    /* LED Driver Functions, mapped to the matter commands and attempt to handle those specific cases */
        esp_err_t set_power(bool power);
        esp_err_t set_brightness(uint8_t brightness);
        esp_err_t set_temperature(uint32_t temperature);
        esp_err_t set_colorXY(long x, long y);
    /* ---------------------------------------------------------------------------------------------- */

    private:
    /* Internal LED Driver Functions */
        esp_err_t set_duty();
        esp_err_t set_channel_duty(float *new_Color, float *old_Color, ledc_channel_t channel);
        esp_err_t generateChannelConfig(uint8_t gpio, uint4_t channel);
        uint32_t  duty_to_pwm(float color);
    /* ---------------------------- */

    private:
        ledc_timer_t timer = LEDC_TIMER_0;
        const char *TAG = "led_driver";
        
    private:
        uint4_t  nChannels = {};
        uint32_t max_pwm   = {};


    private:
        bool power   = {}; // If any of the lights are on
        bool channel_enabled[ESP32C6_MAX_CHANNELS] = {}; // If the channel is enabled or not

        uint8_t bri  = {}; // Bri is the currently applied brightness
        uint8_t nbri = {}; // nBri is the working value to be applied

        RGB_CCT_Duty_t RGB  = {};  // RGB is the currently applied value
        RGB_CCT_Duty_t nRGB = {};  // nRGB is the working values to be applied

        XY_color_t XY = {-1, -1}; // This is the XY value that will be converted to RGB once both values are present
};

/* LEDC Timer Configuration */
ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_SPEED_MODE,           // timer mode
    .duty_resolution = LEDC_TIMER_13_BIT,    // resolution of PWM duty
    .timer_num = LEDC_TIMER,                 // timer index
    .freq_hz = 5000,                         // frequency of PWM signal
    .clk_cfg = LEDC_AUTO_CLK                 // Auto select the source clock
};
/* ------------------------ */


#ifdef __cplusplus
}
#endif
