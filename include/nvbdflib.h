/**
 * nvBDFLib - Easy to use library to handle BDF font files
 * Copyright (c) 2014 Giuseppe Gatta (a.k.a. nextvolume)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NVBDFLIB_NVBDFLIB_H
#define _NVBDFLIB_NVBDFLIB_H

#include <stdio.h>
#define NVBDFLIB_FIELDLEN 4096

typedef struct {
  int w;
  int h;
  int xOff;
  int yOff;
} _BBox;

typedef struct {
  int swx0;
  int swy0;
  int swx1;
  int swy1;
  int dwx0;
  int dwy0;
  int dwx1;
  int dwy1;
  int vXOff;
  int vYOff;
} _Metrics;

typedef struct {
  char name[NVBDFLIB_FIELDLEN];
  int pointSize;
  int xRes;
  int yRes;

  _BBox BBox;
  _Metrics Metrics;

  int chars;
} FontInfo;

typedef struct {
  char name[64];
  int encoding;

  _BBox BBox;
  _Metrics Metrics;
  unsigned char *bitmap;
} FontChar;

typedef struct {
  FontInfo info;
  FontChar *chars;
} BDF_FONT;

/**
 * Returns a pointer to a newly allocated BDF_FONT structure, reading BDF data
 * from a buffer.
 *
 * @param buffer Pointer to buffer
 * @param length Length in bytes
 * @return Pointer to a newly allocated BDF_FONT structure on success, or NULL
 * on failure.
 */

BDF_FONT *bdfReadBuffer(void *dataBuffer, int length);

/**
 * Returns a pointer to a newly allocated, reading BDF data from
 * a zero-terminated string.
 *
 * @param string Pointer to string
 * @return Pointer to a newly allocated BDF_FONT structure on success, or NULL
 * on failure.
 */

BDF_FONT *bdfReadString(char *string);

/**
 * Returns a pointer to a newly allocated BDF_FONT structure, reading BDF data
 * from a file.
 *
 * @param bdfFile Pointer to the FILE structure for a file which can be read
 * @return Pointer to a newly allocated BDF_FONT structure on success, or NULL
 * on failure.
 */

BDF_FONT *bdfReadFile(FILE *bdfFile);

/**
 * Returns a pointer to a newly allocated BDF_FONT structure, reading BDF data
 * from the file at the specified path.
 *
 * @param bdfPath Path to a file
 * @return Pointer to a newly allocated BDF_FONT structure on success, or NULL
 * on failure.
 */

BDF_FONT *bdfReadPath(char *bdfPath);

/**
 * Free the memory that was allocated for a BDF font
 * @param oldFont Pointer to a BDF_FONT structure.
 */

void bdfFree(BDF_FONT *oldFont);

/**
 * Draws a string using the specified font, at the specified coordinates.
 * This function does not draw directly, but actually only calls the function
 * set by bdfSetDrawingFunction() with the right parameters. If no drawing
 * function was set with bdfSetDrawingFunction(), this function has no effect.
 * @param font Pointer to a BDF_FONT structure.
 * @param x Starting X coordinate
 * @param y Starting Y coordinate
 * @param string String
 */

void bdfPrintString(BDF_FONT *font, int x, int y, char *string);

void bdfPrintCharacter(BDF_FONT *font, int x, int y, int character);

/**
 * Set the function delegated to draw.
 * The third parameter ('c') passed to the delegated function
 * is 0 when the pixel to draw is white, and 1 when the pixel to draw
 * is black.
 * The fourth param is a void pointer passed to the function.
 *
 * Applications are free to manage this in the desired manner,
 * for instance, they may not draw a pixel when it is white,
 * therefore considering white transparent.
 *
 * @param drawFunc Drawing function delegated to do the real drawing.
 */

void bdfSetDrawingFunction(void (*drawFunc)(int x, int y, int c, void *ctx), void *ctx);

/**
 * Set the size of the drawing area.
 * The drawing area width and height are just hints, if you know what
 * to do it is perfectly safe to ignore them in your drawing function,
 * except if the height is zero or less than zero, in that case nothing will be
 * drawn; these hints are used in order for the library to provide useful
 * features. For instance width is used by the library to provide some things,
 * like word wrapping.
 *
 * Case study: you will see the bdfbanner example ignore the height
 * parameter, setting it to 1. bdfbanner doesn't use the screen
 * height value because the screen height grows as characters
 * are drawn inside the drawing buffer.
 *
 * @param width Screen width
 * @param height Screen height (if <= 0, nothing will be drawn from this point
 * on)
 */

void bdfSetDrawingAreaSize(int width, int height);

/**
 * Enable or disable word wrap.
 * @param enabled TRUE (1) to enable word wrap, FALSE (0) to disable it.
 */

void bdfSetDrawingWrap(int enabled);

/**
 * Get the current X coordinate after drawing.
 * @return X coordinate
 */

int bdfGetDrawingCurrentX(void);

/**
 * Get the current Y coordinate after drawing.
 * @return Y coordinate
 */

int bdfGetDrawingCurrentY(void);

#endif
