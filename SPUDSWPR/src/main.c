#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <keypadc.h>
#include <graphx.h>
#include <fontlibc.h>
#include <sys/util.h>
#include <sys/timers.h>
#include "gfx/gfx.h"

#include "main.h"
#include "keyboard.h"
#include "clock.h"
#include "minesweeper.h"
#include "misc.h"

const char *errorcodes[] = {
    "",
    "Failed to load DRSANS font! Make sure it's installed.",
};

unsigned int devState;

void prgmEnd(int condition) {
    if(condition) {
        gfx_ZeroScreen();
        gfx_SetTextBGColor(COLOR_Black);
        gfx_SetTextFGColor(COLOR_Red);
        gfx_SetTextConfig(gfx_text_clip);
        char error[222] = "ERROR! ";
        gfx_PrintStringXY(strcat(strcat(error, errorcodes[condition]),"\nPress clear to close."),0,0);
        do {kb_Scan();} while (!kb_IsDown(kb_KeyClear));
    }
    gfx_End();
    exit(condition);
}

uint8_t control_KeyDown = KEY_DOWN;
uint8_t control_KeyLeft = KEY_LEFT;
uint8_t control_KeyRight = KEY_RIGHT;
uint8_t control_KeyUp = KEY_UP;
uint8_t control_KeyFlag = KEY_GRAPH;
uint8_t control_KeyQuestion = KEY_TRACE;
uint8_t control_KeyDig = KEY_ZOOM;

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
        prgmEnd(CONDITION_FONT_LOAD_FAIL);
    }

    kb_Scan();


    minefield_size_x = 12;
    minefield_size_y = 12;
    mine_count = 20;
    seed = 4527;
    initBoard();
}

void loop() {
    curClock = clock();
    keyboardLoop();
    if(keys[KEY_DEL]) { 
        prgmEnd(CONDITION_SUCCESS);
    }

    if(keys[control_KeyDown]) {
        moveCursor(cursor_x, cursor_y >= minefield_size_y - 1 ? 0 : cursor_y + 1);
        
    }
    if(keys[control_KeyRight]) {
        moveCursor(cursor_x >= minefield_size_x - 1 ? 0 : cursor_x + 1, cursor_y);
    }
    if(keys[control_KeyUp]) {
        moveCursor(cursor_x, cursor_y <= 0 ? minefield_size_y - 1 : cursor_y - 1);
    }
    if(keys[control_KeyLeft]) {
        moveCursor(cursor_x <= 0 ? minefield_size_x - 1 : cursor_x - 1, cursor_y);
    }
}

unsigned int oldDevState = 255;

void draw() {
    srandom(100);//random());

    minefield_size_x = 10; //randInt(1,FIELD_MAX_Y);
    minefield_size_y = 10; //randInt(1,FIELD_MAX_Y);
    mine_count = 10; //randInt(1,randInt(1,minefield_size_x*minefield_size_y));
    seed = 1337;
    gfx_ZeroScreen();
    unsigned long preTime = clock();
    initBoard();

    char str[20];
    sprintf(str, "%d", clock()-preTime);
    gfx_PrintStringXY(str,0,0);
}

/* Main function, called first */
int main(void)
{
    prgmStart();

    while(1) {
        loop();
    }
}