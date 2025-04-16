idf_component_register(
    SRCS
        "Adafruit_NeoPixel.cpp"
        "esp.c"
        "esp8266.c"
        "kendyte_k210.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        "arduino"
)
