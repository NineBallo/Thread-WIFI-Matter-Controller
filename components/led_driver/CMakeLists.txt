idf_component_register(SRCS "led_driver.cpp" "color_format.cpp"
                       PRIV_REQUIRES driver esp_driver_ledc
                       INCLUDE_DIRS include)
