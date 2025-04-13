idf_component_register(
    SRCS "ssd1306.c" "ssd1306_fonts.c" "nvbdflib.c"
    INCLUDE_DIRS "include"
    REQUIRES "driver"
)