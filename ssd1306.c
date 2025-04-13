/*
 * SPDX-FileCopyrightText: 2025 Subalpine Circuits
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Copyright 20
#include "ssd1306.h"
#include "driver/i2c_master.h"
#include "nvbdflib.h"
#include "string.h" // for memset

#define SSD1306_WRITE_CMD (0x00)
#define SSD1306_WRITE_DAT (0x40)

#define COORDINATE_SWAP(x1, x2, y1, y2)                                                                                \
  {                                                                                                                    \
    int16_t temp = x1;                                                                                                 \
    x1 = x2, x2 = temp;                                                                                                \
    temp = y1;                                                                                                         \
    y1 = y2;                                                                                                           \
    y2 = temp;                                                                                                         \
  }

typedef struct {
  i2c_master_dev_handle_t i2c_dev_handle;
  uint8_t s_chDisplayBuffer[128][8];
  BDF_FONT *bdf_font;
} ssd1306_dev_t;

static esp_err_t ssd1306_write_data(ssd1306_handle_t dev, const uint8_t *const data, const uint16_t data_len) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;
  esp_err_t ret;

  uint8_t *out_buf = (uint8_t *)calloc(data_len + 1, sizeof(uint8_t));
  out_buf[0] = SSD1306_WRITE_DAT;
  memcpy(out_buf + 1, data, data_len);
  ret = i2c_master_transmit(device->i2c_dev_handle, out_buf, data_len + 1, 1000);
  free(out_buf);

  return ret;
}

static esp_err_t ssd1306_write_cmd(ssd1306_handle_t dev, const uint8_t *const data, const uint16_t data_len) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;
  esp_err_t ret;

  uint8_t *out_buf = (uint8_t *)calloc(data_len + 1, sizeof(uint8_t));
  out_buf[0] = SSD1306_WRITE_CMD;
  memcpy(out_buf + 1, data, data_len);
  ret = i2c_master_transmit(device->i2c_dev_handle, out_buf, data_len + 1, 1000);
  free(out_buf);

  return ret;
}

static inline esp_err_t ssd1306_write_cmd_byte(ssd1306_handle_t dev, const uint8_t cmd) {
  return ssd1306_write_cmd(dev, &cmd, 1);
}

void ssd1306_fill_rectangle(ssd1306_handle_t dev, uint8_t chXpos1, uint8_t chYpos1, uint8_t chXpos2, uint8_t chYpos2,
                            uint8_t chDot) {
  uint8_t chXpos, chYpos;

  for (chXpos = chXpos1; chXpos <= chXpos2; chXpos++) {
    for (chYpos = chYpos1; chYpos <= chYpos2; chYpos++) {
      ssd1306_fill_point(dev, chXpos, chYpos, chDot);
    }
  }
}

void ssd1306_fill_point(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos, uint8_t chPoint) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;
  uint8_t chPos, chBx, chTemp = 0;

  if (chXpos > 127 || chYpos > 63) {
    return;
  }
  chPos = 7 - chYpos / 8;
  chBx = chYpos % 8;
  chTemp = 1 << (7 - chBx);

  if (chPoint) {
    device->s_chDisplayBuffer[chXpos][chPos] |= chTemp;
  } else {
    device->s_chDisplayBuffer[chXpos][chPos] &= ~chTemp;
  }
}

void ssd1306_draw_bitmap(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos, const uint8_t *pchBmp, uint8_t chWidth,
                         uint8_t chHeight) {
  uint16_t i, j, byteWidth = (chWidth + 7) / 8;

  for (j = 0; j < chHeight; j++) {
    for (i = 0; i < chWidth; i++) {
      if (*(pchBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        ssd1306_fill_point(dev, chXpos + i, chYpos + j, 1);
      }
    }
  }
}

void ssd1306_draw_line(ssd1306_handle_t dev, int16_t chXpos1, int16_t chYpos1, int16_t chXpos2, int16_t chYpos2) {
  // 16-bit variables allowing a display overflow effect
  int16_t x_len = abs(chXpos1 - chXpos2);
  int16_t y_len = abs(chYpos1 - chYpos2);

  if (y_len < x_len) {
    if (chXpos1 > chXpos2) {
      COORDINATE_SWAP(chXpos1, chXpos2, chYpos1, chYpos2);
    }
    int16_t len = x_len;
    int16_t diff = y_len;

    do {
      if (diff >= x_len) {
        diff -= x_len;
        if (chYpos1 < chYpos2) {
          chYpos1++;
        } else {
          chYpos1--;
        }
      }

      diff += y_len;
      ssd1306_fill_point(dev, chXpos1++, chYpos1, 1);
    } while (len--);
  }

  else {
    if (chYpos1 > chYpos2) {
      COORDINATE_SWAP(chXpos1, chXpos2, chYpos1, chYpos2);
    }
    int16_t len = y_len;
    int16_t diff = x_len;

    do {
      if (diff >= y_len) {
        diff -= y_len;
        if (chXpos1 < chXpos2) {
          chXpos1++;
        } else {
          chXpos1--;
        }
      }

      diff += x_len;
      ssd1306_fill_point(dev, chXpos1, chYpos1++, 1);
    } while (len--);
  }
}

void bdf_drawing_function(int x, int y, int c, void *ctx) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)ctx;
  ssd1306_fill_point(device, x, y, c);
}

esp_err_t ssd1306_load_bdf_buffer(ssd1306_handle_t dev, void *buffer, int length, bool wrap) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;

  if (!(device->bdf_font = bdfReadBuffer(buffer, length))) {
    return ESP_FAIL;
  }

  bdfSetDrawingFunction(bdf_drawing_function, (void *)device);
  bdfSetDrawingAreaSize(128, 64);
  bdfSetDrawingWrap(wrap);

  return ESP_OK;
};

esp_err_t ssd1306_load_bdf_file(ssd1306_handle_t dev, FILE *file, bool wrap) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;

  if (!(device->bdf_font = bdfReadFile(file))) {
    return ESP_FAIL;
  }

  bdfSetDrawingFunction(bdf_drawing_function, (void *)device);
  bdfSetDrawingAreaSize(128, 64);
  bdfSetDrawingWrap(wrap);

  return ESP_OK;
};

void ssd1306_draw_bdf_text(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos, char *string) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;
  bdfPrintString(device->bdf_font, chXpos, chYpos, string);
};

esp_err_t ssd1306_init(ssd1306_handle_t dev) {
  esp_err_t ret;

  ssd1306_write_cmd_byte(dev, 0xAE); //--turn off oled panel
  ssd1306_write_cmd_byte(dev, 0x40); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  ssd1306_write_cmd_byte(dev, 0x81); //--set contrast control register
  ssd1306_write_cmd_byte(dev, 0xCF); // Set SEG Output Current Brightness
  ssd1306_write_cmd_byte(dev, 0xA1); //--Set SEG/Column Mapping
  ssd1306_write_cmd_byte(dev, 0xC0); // Set COM/Row Scan Direction
  ssd1306_write_cmd_byte(dev, 0xA6); //--set normal display
  ssd1306_write_cmd_byte(dev, 0xA8); //--set multiplex ratio(1 to 64)
  ssd1306_write_cmd_byte(dev, 0x3f); //--1/64 duty
  ssd1306_write_cmd_byte(dev, 0xd5); //--set display clock divide ratio/oscillator frequency
  ssd1306_write_cmd_byte(dev, 0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
  ssd1306_write_cmd_byte(dev, 0xD9); //--set pre-charge period
  ssd1306_write_cmd_byte(dev, 0xF1); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  ssd1306_write_cmd_byte(dev, 0xDA); //--set com pins hardware configuration
  ssd1306_write_cmd_byte(dev, 0xDB); //--set vcomh
  ssd1306_write_cmd_byte(dev, 0x40); // Set VCOM Deselect Level
  ssd1306_write_cmd_byte(dev, 0x8D); //--set Charge Pump enable/disable
  ssd1306_write_cmd_byte(dev, 0x14); //--set(0x10) disable
  ssd1306_write_cmd_byte(dev, 0xA4); // Disable Entire Display On (0xa4/0xa5)
  ssd1306_write_cmd_byte(dev, 0xA6); // Disable Inverse Display On (0xa6/a7)

  const uint8_t cmd[2] = {0x20, 1}; //-- set vertical adressing mode
  ssd1306_write_cmd(dev, cmd, sizeof(cmd));

  uint8_t cmd2[3] = {0x21, 0, 127};
  ssd1306_write_cmd(dev, cmd2, sizeof(cmd2)); //--set column address to zero
  cmd2[0] = 0x22;
  cmd2[2] = 7;
  ssd1306_write_cmd(dev, cmd2, sizeof(cmd2)); //--set row address to zero

  ret = ssd1306_write_cmd_byte(dev, 0xAF); //--turn on oled panel
  ret = 0;

  ssd1306_clear_screen(dev, 0x00);
  return ret;
}

ssd1306_handle_t ssd1306_create(i2c_master_dev_handle_t i2c_dev_handle) {
  ssd1306_dev_t *dev = (ssd1306_dev_t *)calloc(1, sizeof(ssd1306_dev_t));
  dev->i2c_dev_handle = i2c_dev_handle;
  ssd1306_init((ssd1306_handle_t)dev);
  return (ssd1306_handle_t)dev;
}

void ssd1306_delete(ssd1306_handle_t dev) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;
  free(device);
}

esp_err_t ssd1306_refresh_gram(ssd1306_handle_t dev) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;
  return ssd1306_write_data(dev, &device->s_chDisplayBuffer[0][0], sizeof(device->s_chDisplayBuffer));
}

void ssd1306_clear_screen(ssd1306_handle_t dev, uint8_t chFill) {
  ssd1306_dev_t *device = (ssd1306_dev_t *)dev;
  memset(device->s_chDisplayBuffer, chFill, sizeof(device->s_chDisplayBuffer));
}