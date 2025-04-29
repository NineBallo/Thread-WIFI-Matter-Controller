idf_component_register( SRC_DIRS "."
                        PRIV_REQUIRES esp_event nvs_flash esp_matter led_driver
                        INCLUDE_DIRS ".")

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 23)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
