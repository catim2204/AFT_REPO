idf_component_register(SRCS "hello_world_main.c"
                       PRIV_REQUIRES spi_flash bt nvs_flash driver esp_wifi
                       INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../Shared/Inc)
                       
