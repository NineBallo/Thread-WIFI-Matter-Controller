#include <stdio.h>
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <esp_matter.h>
#include <app_priv.h>

#include <led_driver.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";

static LED_Driver *LED_Interface = nullptr;

extern uint16_t light_endpoint_id;

/* Convert/Remap values then pass to the led driver or misc hardware interface */
static esp_err_t app_driver_light_set_power(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    return LED_Interface.set_power(val->val.b);
}
static esp_err_t app_driver_light_set_brightness(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_BRIGHTNESS, STANDARD_BRIGHTNESS);
    return LED_Interface.set_brightness(value);
}
static esp_err_t app_driver_light_set_temperature(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    uint32_t value = REMAP_TO_RANGE_INVERSE(val->val.u16, STANDARD_TEMPERATURE_FACTOR);
    return LED_Interface.set_temperature(value);
}
static esp_err_t app_driver_light_set_x(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    return LED_Interface.set_colorXY(static_cast<long>(val->val.u32), -1);
}
static esp_err_t app_driver_light_set_y(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    return LED_Interface.set_colorXY(-1, static_cast<long>(val->val.u32));
}
/*----------------------------------------------------------------------------*/


/* Callback that runs whenever an attribute updates. Calls the respective handler for that type of update */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_endpoint_id) {
        led_driver_handle_t handle = (led_driver_handle_t)driver_handle;
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                err = app_driver_light_set_power(handle, val);
            }
        } else if (cluster_id == LevelControl::Id) {
            if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
                err = app_driver_light_set_brightness(handle, val);
            }
        } else if (cluster_id == ColorControl::Id) {
            if (attribute_id == ColorControl::Attributes::ColorTemperatureMireds::Id) {
                err = app_driver_light_set_temperature(handle, val);
            } else if (attribute_id == ColorControl::Attributes::CurrentX::Id) {
                err = app_driver_light_set_x(handle, val);
            } else if (attribute_id == ColorControl::Attributes::CurrentY::Id) {
                err = app_driver_light_set_y(handle, val);
            }
        }
    }
    return err;
}
/* ------------------------------------------------------------------------------------------------------ */

/* Sets the initial default values at startup or any initilization. Doesnt matter too much from my experience */
esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    void *priv_data = endpoint::get_priv_data(endpoint_id);
    led_driver_handle_t handle = (led_driver_handle_t)priv_data;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    
    /* Setting brightness */
    attribute_t *attribute = attribute::get(endpoint_id, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_brightness(handle, &val);

    /* Setting color */
    attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::ColorMode::Id);
    attribute::get_val(attribute, &val);

    if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kColorTemperature) {
        /* Setting temperature */
        attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id);
        attribute::get_val(attribute, &val);
        err |= app_driver_light_set_temperature(handle, &val);
    }

    else if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kCurrentXAndCurrentY) {
        //no default lol
    }

    else {
        ESP_LOGE(TAG, "Color mode not supported");
    }

    /* Setting power */
    attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_power(handle, &val);

    return err;
}
/* ---------------------------------------------------------------------------------------------------------- */

/* Initialize everything needed for the hardware/software layers */
app_driver_handle_t app_driver_light_init()
{

    /* Initialize Hardware Layer */
    LED_GPIO_MAP map = {};
    map.red   = PIN_R;
    map.green = PIN_G;
    map.blue  = PIN_B;
    map.white = PIN_W;
    map.warmwhite = PIN_WW;

    LED_Interface = new LED_Driver(map); // Allocate the instance
    
    return (app_driver_handle_t)nullptr;
}