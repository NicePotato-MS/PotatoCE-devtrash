#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <sys/timers.h>

#include <keypadc.h>
#include <graphx.h>
#include <fontlibc.h>

#include "main.h"
#include "gfx/gfx.h"


kb_key_t key;

uint8_t old_kb_Data[8]; 

unsigned char cursor_x;
unsigned char cursor_y;
unsigned char dev_state;


void prgmStart() {
    gfx_Begin();
    gfx_ZeroScreen();

    fontlib_font_t *font;

    gfx_SetPalette(main_palette, sizeof_main_palette, 0);
    gfx_SetTextTransparentColor(COLOR_Transparent);
    gfx_SetTextFGColor(COLOR_Gray);
    gfx_SetTextBGColor(COLOR_Black);

    // Setup font
    font = fontlib_GetFontByIndex("DRSANS", 3);
    if (!font) {
        gfx_PrintStringXY("DRSANS appvar not found or invalid",2,20);
        do {kb_Scan();} while (!kb_IsDown(kb_KeyClear));
        prgmEnd(CONDITION_FONT_LOAD_FAIL);
    }

    kb_Scan();
}

void prgmEnd(int condition) {
    gfx_End();
    exit(condition);
}

void loop() {
    memcpy(old_kb_Data,kb_Data,sizeof(old_kb_Data));
    kb_Scan();

    key = kb_Data[7];
    

}

void draw() {
    
}

/* Main function, called first */
int main(void)
{
    prgmStart();

    while(1) {
        loop();
        draw();
    }
}