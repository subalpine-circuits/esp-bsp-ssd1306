# Component: SSD1306

Port of https://github.com/nopnop2002/esp-idf-ssd1306/tree/master/components/ssd1306 upgraded to the 5.2.x I2C master driver.

## C example
```C
#include <stdio.h>
#include "ssd1306.h"
#include "ssd1306.h"

#define I2C_MASTER_SCL_IO 26        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 25        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define SSD1306_ADDRESS 0x3C        /*!< OLED address */

static ssd1306_handle_t ssd1306_dev = NULL;

void app_main(void)
{
  i2c_master_bus_config_t i2c_mst_config = {
      .i2c_port = I2C_MASTER_NUM,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags = {.enable_internal_pullup = true},
  };
  i2c_master_bus_handle_t i2c_bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle));

  i2c_master_bus_handle_t i2c_bus_handle;
  i2c_master_get_bus_handle(0, &i2c_bus_handle);

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = SSD1306_ADDRESS,
      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
  };
  i2c_master_dev_handle_t i2c_dev_handle;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle));

  display = ssd1306_create(i2c_dev_handle);

  char data_str[10] = {0};
  sprintf(data_str, "C STR");
  ssd1306_draw_string(display, 70, 16, (const uint8_t *)data_str, 16, 1);
  ssd1306_refresh_gram(display);
}
```