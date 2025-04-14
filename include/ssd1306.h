/*
 * SPDX-FileCopyrightText: 2025 Subalpine Circuits
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief SSD1306 driver
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c_master.h"
#include "stdint.h"

/**
 * @brief  I2C address.
 */
#define SSD1306_I2C_ADDRESS ((uint8_t)0x3C)

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

typedef void *ssd1306_handle_t; /*handle of ssd1306*/

/**
 * @brief   device initialization
 *
 * @param   dev object handle of ssd1306
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t ssd1306_init(ssd1306_handle_t dev);

/**
 * @brief   Create and initialization device object and return a device handle
 *
 * @param   i2c device handle
 *
 * @return
 *     - device object handle of ssd1306
 */
ssd1306_handle_t ssd1306_create(i2c_master_dev_handle_t i2c_dev_handle);

/**
 * @brief   Delete and release a device object
 *
 * @param   dev object handle of ssd1306
 */
void ssd1306_delete(ssd1306_handle_t dev);

/**
 * @brief   draw point on (x, y)
 *
 * @param   dev object handle of ssd1306
 * @param   chXpos Specifies the X position
 * @param   chYpos Specifies the Y position
 * @param   chPoint fill point
 */
void ssd1306_fill_point(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos,
                        uint8_t chPoint);

/**
 * @brief   Draw rectangle on (x1,y1)-(x2,y2)
 *
 * @param   dev object handle of ssd1306
 * @param   chXpos1
 * @param   chYpos1
 * @param   chXpos2
 * @param   chYpos2
 * @param   chDot fill point
 */
void ssd1306_fill_rectangle(ssd1306_handle_t dev, uint8_t chXpos1,
                            uint8_t chYpos1, uint8_t chXpos2, uint8_t chYpos2,
                            uint8_t chDot);

/**
 * @brief   draw bitmap on (x, y),and set width, height
 *
 * @param   dev object handle of ssd1306
 * @param   chXpos Specifies the X position
 * @param   chYpos Specifies the Y position
 * @param   pchBmp point to BMP data
 * @param   chWidth picture width
 * @param   chHeight picture heght
 */
void ssd1306_draw_bitmap(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos,
                         const uint8_t *pchBmp, uint8_t chWidth,
                         uint8_t chHeight);

/**
 * @brief   draw line between two specified points
 *
 * @param   dev object handle of ssd1306
 * @param   chXpos1 Specifies the X position of the starting point of the line
 * @param   chYpos1 Specifies the Y position of the starting point of the line
 * @param   chXpos2 Specifies the X position of the ending point of the line
 * @param   chYpos2 Specifies the Y position of the ending point of the line
 */
void ssd1306_draw_line(ssd1306_handle_t dev, int16_t chXpos1, int16_t chYpos1,
                       int16_t chXpos2, int16_t chYpos2);

/**
 * @brief   load a BDF font via buffer
 *
 * @param   dev object handle of ssd1306
 * @param   buffer pointer to buffer
 * @param   length length of buffer
 * @param   wrap whether text should wrap
 */
esp_err_t ssd1306_load_bdf_buffer(ssd1306_handle_t dev, void *buffer,
                                  int length, bool wrap);

/**
 * @brief   load a BDF font via file
 *
 * @param   dev object handle of ssd1306
 * @param   file font file
 * @param   wrap whether text should wrap
 */
esp_err_t ssd1306_load_bdf_file(ssd1306_handle_t dev, FILE *file, bool wrap);

/**
 * @brief   draw text using BDF font
 *
 * @param   dev object handle of ssd1306
 * @param   chXpos x coord
 * @param   chYpos y coord
 * @param   string string to draw
 */
void ssd1306_draw_bdf_text(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos,
                           const char *string);

/**
 * @brief   refresh dot matrix panel
 *
 * @param   dev object handle of ssd1306

 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 **/
esp_err_t ssd1306_refresh_gram(ssd1306_handle_t dev);

/**
 * @brief   Clear screen
 *
 * @param   dev object handle of ssd1306
 * @param   chFill whether fill and fill char
 **/
void ssd1306_clear_screen(ssd1306_handle_t dev, uint8_t chFill);

#ifdef __cplusplus
}
#endif