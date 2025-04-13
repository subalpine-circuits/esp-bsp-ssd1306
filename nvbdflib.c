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

#include "nvbdflib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FIELDLEN NVBDFLIB_FIELDLEN

static char stringBuf[0x1000];
static char cmdName[64];

static struct {
  void (*function)(int x, int y, int c, void *ctx);
  int areaWidth;
  int areaHeight;
  int wrap;
  int currentX;
  int currentY;
  void *ctx;
} bdfDraw = {.function = NULL, .areaWidth = 0, .areaHeight = 0, .wrap = 0, .currentX = 0, .currentY = 0, .ctx = NULL};

/**
 * Visopsys' strlen() returns an error if it goes farther
 * than MAXSTRINGLENGTH (usually 4096) bytes from the string pointer given.
 * This strange implementation was probably intended to try to
 * avoid segfaults (but it does a bad job at that, too), this is more like a bug than a feature.
 * So, use our own strlen in place of Visopsys' one
 */

#ifdef __VISOPSYS__

#define strlen(string) fixed_strlen(string)

static size_t fixed_strlen(const char *string) {
  int count;

  for (count = 0; string[count]; count++)
    ;

  return count;
}

#endif

BDF_FONT *bdfReadBuffer(void *dataBuffer, int length) {
  BDF_FONT *newFont;
  char *strPtr;
  char *strPtr2;
  int *intPtr = NULL;
  int element = 0;
  int curChar = 0;
  int x, l;
  int endFont = 0;
  int curBitmapPos = 0;
  int isBitmap = 0;
  char hexBuf[3];
  unsigned int hex;
  unsigned int bitmapSize;
  int bufPos = 0;
  char *buffer = dataBuffer;

  newFont = malloc(sizeof(BDF_FONT));

  if (newFont == NULL)
    return NULL;

  bzero(newFont, sizeof(BDF_FONT));

  while (bufPos < length && !endFont) {
    element = 0;

    x = 0;

    do {
      stringBuf[x++] = buffer[bufPos];

      if (buffer[bufPos] == '\n') {
        bufPos++;
        break;
      } else
        bufPos++;
    } while (x < (sizeof(stringBuf) - 1) && buffer[bufPos] && bufPos < length);

    stringBuf[x] = '\0';

    // Remove newline and carriage return.
    strPtr = stringBuf;

    while (*strPtr) {
      if (*strPtr == '\n')
        *strPtr = 0;
      else if (*strPtr == '\r')
        *strPtr = 0;

      strPtr++;
    }

    strPtr2 = stringBuf;

    while ((strPtr = strtok(strPtr2, " "))) {
      if (element == 0) {
        strncpy(cmdName, strPtr, 63);
        cmdName[63] = 0;
      }

      if (newFont->info.chars == 0) {
        if (strcasecmp(cmdName, "FONT") == 0) {
          if (element == 1) {
            strncpy(newFont->info.name, strPtr, FIELDLEN - 1);
            newFont->info.name[FIELDLEN - 1] = 0;
          }
        } else if (strcasecmp(cmdName, "SIZE") == 0) {
          if (element == 1) // PointSize
            sscanf(strPtr, "%d", &newFont->info.pointSize);
          else if (element == 2) // Xres
            sscanf(strPtr, "%d", &newFont->info.xRes);
          else if (element == 3)
            sscanf(strPtr, "%d", &newFont->info.yRes);
        } else if (strcasecmp(cmdName, "FONTBOUNDINGBOX") == 0) {
          switch (element) {
          case 1: //  FBBx
            intPtr = &newFont->info.BBox.w;
            break;
          case 2: // FBBy
            intPtr = &newFont->info.BBox.h;
            break;
          case 3: // Xoff
            intPtr = &newFont->info.BBox.xOff;
            break;
          case 4: // Yoff
            intPtr = &newFont->info.BBox.yOff;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "SWIDTH") == 0) {
          switch (element) {
          case 1: // swx0
            intPtr = &newFont->info.Metrics.swx0;
            break;
          case 2: // swy0
            intPtr = &newFont->info.Metrics.swy0;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "DWIDTH") == 0) {
          switch (element) {
          case 1: // dwx0
            intPtr = &newFont->info.Metrics.dwx0;
            break;
          case 2: // dwy0
            intPtr = &newFont->info.Metrics.dwy0;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "SWIDTH1") == 0) {
          switch (element) {
          case 1: // swx1
            intPtr = &newFont->info.Metrics.swx1;
            break;
          case 2: // swy1
            intPtr = &newFont->info.Metrics.swy1;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "DWIDTH1") == 0) {
          switch (element) {
          case 1: // dwx1
            intPtr = &newFont->info.Metrics.dwx1;
            break;
          case 2: // dwy1
            intPtr = &newFont->info.Metrics.dwy1;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "VVECTOR") == 0) {
          switch (element) {
          case 1: // xoff
            intPtr = &newFont->info.Metrics.vXOff;
            break;
          case 2: // yoff
            intPtr = &newFont->info.Metrics.vYOff;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "CHARS") == 0) {
          if (element == 1) {
            sscanf(strPtr, "%d", &newFont->info.chars);

            newFont->chars = malloc(newFont->info.chars * sizeof(FontChar));
            curChar = 0;

            for (x = 0; x < newFont->info.chars; x++) {
              memcpy(&newFont->chars[x].BBox, &newFont->info.BBox, sizeof(_BBox));
              memcpy(&newFont->chars[x].Metrics, &newFont->info.Metrics, sizeof(_Metrics));
            }
          }
        }
      } else if (newFont->info.chars > 0) {

        if (strcasecmp(cmdName, "STARTCHAR") == 0) {
          if (element == 1) {
            strncpy(newFont->chars[curChar].name, strPtr, 63);
            newFont->chars[curChar].name[63] = 0;
            isBitmap = 0;
          }
        } else if (strcasecmp(cmdName, "ENCODING") == 0) {
          if (element == 1)
            sscanf(strPtr, "%d", &newFont->chars[curChar].encoding);
        } else if (strcasecmp(cmdName, "BBX") == 0) {
          switch (element) {
          case 1: //  FBBx
            intPtr = &newFont->chars[curChar].BBox.w;
            break;
          case 2: // FBBy
            intPtr = &newFont->chars[curChar].BBox.h;
            break;
          case 3: // Xoff
            intPtr = &newFont->chars[curChar].BBox.xOff;
            break;
          case 4: // Yoff
            intPtr = &newFont->chars[curChar].BBox.yOff;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "SWIDTH") == 0) {
          switch (element) {
          case 1: // swx0
            intPtr = &newFont->chars[curChar].Metrics.swx0;
            break;
          case 2: // swy0
            intPtr = &newFont->chars[curChar].Metrics.swy0;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "DWIDTH") == 0) {
          switch (element) {
          case 1: // dwx0
            intPtr = &newFont->chars[curChar].Metrics.dwx0;
            break;
          case 2: // dwy0
            intPtr = &newFont->chars[curChar].Metrics.dwy0;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "SWIDTH1") == 0) {
          switch (element) {
          case 1: // swx1
            intPtr = &newFont->chars[curChar].Metrics.swx1;
            break;
          case 2: // swy1
            intPtr = &newFont->chars[curChar].Metrics.swy1;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "DWIDTH1") == 0) {
          switch (element) {
          case 1: // dwx1
            intPtr = &newFont->chars[curChar].Metrics.dwx1;
            break;
          case 2: // dwy1
            intPtr = &newFont->chars[curChar].Metrics.dwy1;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "VVECTOR") == 0) {
          switch (element) {
          case 1: // xoff
            intPtr = &newFont->chars[curChar].Metrics.vXOff;
            break;
          case 2: // yoff
            intPtr = &newFont->chars[curChar].Metrics.vYOff;
            break;
          }

          if (intPtr)
            sscanf(strPtr, "%d", intPtr);
        } else if (strcasecmp(cmdName, "BITMAP") == 0) {
          x = newFont->chars[curChar].BBox.w;
          // Pad size to byte size...
          if (x & 7) {
            x |= 7;
            x++;
          }

          bitmapSize = newFont->chars[curChar].BBox.h * (x / 8);

          newFont->chars[curChar].bitmap = malloc(bitmapSize);

          bzero(newFont->chars[curChar].bitmap, bitmapSize);

          curBitmapPos = 0;
          isBitmap = 1;
        } else if (strcasecmp(cmdName, "ENDCHAR") == 0) {
          curChar++;
        } else if (strcasecmp(cmdName, "ENDFONT") == 0) {
          endFont = 1;
        } else {
          if (isBitmap) {
            for (x = 0, l = strlen(strPtr); x < l; x += 2) {
              hexBuf[0] = strPtr[x];
              hexBuf[1] = strPtr[x + 1];
              hexBuf[2] = 0;
              // FontChars[curChar].bitmap[curBitmapPos++] =
              sscanf(hexBuf, "%x", &hex);

              if (curBitmapPos < bitmapSize)
                newFont->chars[curChar].bitmap[curBitmapPos++] = hex;
            }
          }
        }
      }

      element++;
      strPtr2 = NULL;
      intPtr = NULL;
    }
  }

  return newFont;
}

BDF_FONT *bdfReadString(char *string) { return bdfReadBuffer(string, strlen(string)); }

BDF_FONT *bdfReadFile(FILE *bdfFile) {
  BDF_FONT *r;
  int fsize;
  char *buffer;

#ifdef __VISOPSYS__
  fsize = bdfFile->f.size;
#else
  fseek(bdfFile, 0, SEEK_END);
  fsize = ftell(bdfFile);
  fseek(bdfFile, 0, SEEK_SET);
#endif

  buffer = malloc(fsize);

  if (buffer == NULL)
    return NULL;

  fread(buffer, sizeof(char), fsize, bdfFile);

  r = bdfReadBuffer(buffer, fsize);

  free(buffer);

  return r;
}

BDF_FONT *bdfReadPath(char *bdfPath) {
  FILE *bdfFile;
  BDF_FONT *r;

  if (!(bdfFile = fopen(bdfPath, "rb")))
    return NULL;

  r = bdfReadFile(bdfFile);

  fclose(bdfFile);

  return r;
}

void bdfFree(BDF_FONT *oldFont) {
  int i;

  if (oldFont == NULL)
    return;

  // Free the memory for character bitmaps first!

  for (i = 0; i < oldFont->info.chars; i++)
    free(oldFont->chars[i].bitmap);

  // Free the memory for array of character structures.
  free(oldFont->chars);

  // Free the memory for the structure
  free(oldFont);
}

void bdfPrintString(BDF_FONT *font, int x, int y, char *string) {
  while (*string) {
    bdfPrintCharacter(font, x, y, *string);
    x = bdfDraw.currentX;
    y = bdfDraw.currentY;
    string++;
  }
}

void bdfPrintCharacter(BDF_FONT *font, int x, int y, int character) {
  FontChar *ch = NULL;
  int ch_w;
  int ch_h;
  int ch_xoff;
  int ch_yoff;
  int x1, y1, i;
  int cnt;

  int f_h = font->info.BBox.h;
  int f_yoff = font->info.BBox.yOff;

  if (bdfDraw.function == NULL || bdfDraw.areaWidth <= 0 || bdfDraw.areaHeight <= 0)
    return;

  if (character == '\n') {
    x = 0;
    y += f_h;
  }

  ch = NULL;

  for (i = 0; i < font->info.chars; i++) {
    if (font->chars[i].encoding == character) {
      ch = &font->chars[i];
      break;
    }
  }

  if (ch) {
    ch_w = ch->BBox.w;
    ch_h = ch->BBox.h;
    ch_xoff = ch->BBox.xOff;
    ch_yoff = ch->BBox.yOff;
    cnt = 0;

    if (x + ch_xoff + ch_w >= bdfDraw.areaWidth && bdfDraw.wrap) {
      x = 0;
      y += f_h;
    }

    if (ch_w & 7)
      ch_w = (ch_w | 7) + 1;

    for (y1 = 0, cnt = 0; y1 < ch_h; y1++) {
      for (x1 = 0; x1 < ch_w; x1++, cnt++) {
        bdfDraw.function(x + ch_xoff + x1, (y + (f_h - ch_h) + f_yoff + y1) - ch_yoff,
                         (ch->bitmap[cnt / 8] & (0x80 >> (cnt % 8))) > 0, bdfDraw.ctx);
      }
    }

    x += ch->Metrics.dwx0;
  }

  bdfDraw.currentX = x;
  bdfDraw.currentY = y;
}

void bdfSetDrawingFunction(void (*drawFunc)(int x, int y, int c, void *ctx), void *ctx) {
  bdfDraw.function = drawFunc;
  bdfDraw.ctx = ctx;
}

void bdfSetDrawingAreaSize(int width, int height) {
  bdfDraw.areaWidth = width;
  bdfDraw.areaHeight = height;
}

void bdfSetDrawingWrap(int enabled) { bdfDraw.wrap = enabled; }

int bdfGetDrawingCurrentX(void) { return bdfDraw.currentX; }

int bdfGetDrawingCurrentY(void) { return bdfDraw.currentY; }
