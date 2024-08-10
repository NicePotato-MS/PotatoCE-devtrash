#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <graphx.h>
#include "pfonts/drmono_10_regular.h"
#include "pfonts/drmono_10_bold.h"

#include "main.h"

unsigned char* currentFont = NULL;
unsigned char* fontRegular10 = NULL;
unsigned char* fontBold10 = NULL;

void decompressFont(unsigned char* font, unsigned char rawFont[]) {
    unsigned int glyphIndex = 0;
    for(unsigned int i=0; i<sizeof(rawFont);i++) {
        for(unsigned char bit=7; bit>=0; bit--) {
            font[glyphIndex] = ((1 << bit) & rawFont[i]) ? 1:0;
            glyphIndex++;
        }
    }
}

void potatofont_ReadyFonts() {
    if(fontRegular10 != NULL) {free(fontRegular10);}
    if(fontBold10 != NULL) {free(fontBold10);}
    fontRegular10 = malloc(256*DRMONO_10_REGULAR_X_SIZE*DRMONO_10_REGULAR_Y_SIZE);
    if(fontRegular10 == NULL) {prgmEnd(1);}
    fontBold10 = malloc(256*DRMONO_10_BOLD_X_SIZE*DRMONO_10_BOLD_Y_SIZE);
    if(fontBold10 == NULL) {prgmEnd(1);}
    decompressFont(fontRegular10, drmono_10_regular);
    decompressFont(fontBold10, drmono_10_bold);
}

unsigned char potatofont_FGColor;
unsigned char potatofont_BGColor;

void potatofont_DrawGlyph5x10(unsigned char glyph, char x, unsigned int y) {
    unsigned char* vramSpot = gfx_vram+(1+(x*6)+(y*11*GFX_LCD_WIDTH));
    unsigned int fontIndex = glyph*5*10;
    for(unsigned int gy=0; gy<GFX_LCD_WIDTH*10; gy=gy+GFX_LCD_WIDTH) {
        for(unsigned char gx=0; gx<5; gx++) {
            // glyphX+glyphY
            vramSpot[gx+gy] = currentFont[fontIndex];
            fontIndex++;
        }
    }
}