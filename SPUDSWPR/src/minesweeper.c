#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <graphx.h>
#include <sys/util.h>

#include "main.h"
#include "gfx/gfx.h"
#include "minesweeper.h"
#include "misc.h"

unsigned char *sprites[] = { // Sorry for this atrocity
    (unsigned char*)((uintptr_t)&tile_down_data + 2),             //0
    (unsigned char*)((uintptr_t)&tile_1_data + 2),              //1
    (unsigned char*)((uintptr_t)&tile_2_data + 2),              //2
    (unsigned char*)((uintptr_t)&tile_3_data + 2),              //3
    (unsigned char*)((uintptr_t)&tile_4_data + 2),              //4
    (unsigned char*)((uintptr_t)&tile_5_data + 2),              //5
    (unsigned char*)((uintptr_t)&tile_6_data + 2),              //6
    (unsigned char*)((uintptr_t)&tile_7_data + 2),              //7
    (unsigned char*)((uintptr_t)&tile_8_data + 2),              //8
    (unsigned char*)((uintptr_t)&tile_up_data + 2),           //9
    (unsigned char*)((uintptr_t)&tile_flag_data + 2),           //10
    (unsigned char*)((uintptr_t)&tile_flagerror_data + 2),      //11
    (unsigned char*)((uintptr_t)&tile_question_data + 2),       //12
    (unsigned char*)((uintptr_t)&tile_questiondown_data + 2),   //13
    (unsigned char*)((uintptr_t)&tile_bomb_data + 2),           //14
    (unsigned char*)((uintptr_t)&tile_hitbomb_data + 2),        //15
    (unsigned char*)((uintptr_t)&tile_nobomb_data + 2),         //16
    (unsigned char*)((uintptr_t)&ui_0_data + 2),                //17
    (unsigned char*)((uintptr_t)&ui_1_data + 2),                //18
    (unsigned char*)((uintptr_t)&ui_2_data + 2),                //19
    (unsigned char*)((uintptr_t)&ui_3_data + 2),                //20
    (unsigned char*)((uintptr_t)&ui_4_data + 2),                //21
    (unsigned char*)((uintptr_t)&ui_5_data + 2),                //22
    (unsigned char*)((uintptr_t)&ui_6_data + 2),                //23
    (unsigned char*)((uintptr_t)&ui_7_data + 2),                //24
    (unsigned char*)((uintptr_t)&ui_8_data + 2),                //25
    (unsigned char*)((uintptr_t)&ui_9_data + 2),                //26
    (unsigned char*)((uintptr_t)&face_happy_data + 2),          //27
    (unsigned char*)((uintptr_t)&face_shock_data + 2),          //28
    (unsigned char*)((uintptr_t)&face_cool_data + 2),           //29
    (unsigned char*)((uintptr_t)&face_dead_data + 2),           //30
    (unsigned char*)((uintptr_t)&face_down_data + 2)            //31
};

unsigned int draw_start_x;
unsigned int draw_start_y;

unsigned char minefield_data[FIELD_MAX_X*FIELD_MAX_Y];
unsigned char minefield_state[FIELD_MAX_X*FIELD_MAX_Y];
int minefield_size_x;
int minefield_size_y;
int mine_count;
int cursor_x = 0;
int cursor_y = 0;
clock_t cursor_lastMove = 0;
bool cursorHasMoved = false;
int lastCursorMove = 0;
int seed;
int game_start;

void renderTile(unsigned char *sprite, uint8_t location[]) {
    // Copy sprite data into location
    unsigned int offset;
    offset = 0;
    for (uint8_t x = 0; x<16; x++) {
        for (uint8_t y = 0; y<16; y++) {
            location[x*GFX_LCD_WIDTH+y] = sprite[offset];
            offset++;
        }
    }
    
}

void displayTile(unsigned x,unsigned y) {
    renderTile(sprites[minefield_state[x+y*minefield_size_x]],&gfx_vram[draw_start_x+x*16+(draw_start_y+y*16)*GFX_LCD_WIDTH]);
}

unsigned int tileVRAMX(unsigned int x) {
    return draw_start_x+x*16;
}

unsigned int tileVRAMY(unsigned int y) {
    return draw_start_y+y*16;
}

void moveCursor(unsigned int x, unsigned int y) {
    displayTile(cursor_x,cursor_y);
    gfx_SetColor(COLOR_Black);
    gfx_Rectangle_NoClip(draw_start_x+x*16, draw_start_y+y*16, 16, 16);
    cursor_x = x;
    cursor_y = y;
}

void drawBoard() {
    unsigned int offset;
    offset = 0;
    for(unsigned int y = draw_start_y; y < (minefield_size_y*16) + draw_start_y; y = y+16) {
        for(unsigned int x = draw_start_x; x < (minefield_size_x*16) + draw_start_x; x = x+16) {
            renderTile(sprites[minefield_state[offset]],&gfx_vram[x+y*GFX_LCD_WIDTH]);
            offset++;
        }
    }
}

unsigned char tile;

void initBoard() {
    memset(minefield_data, TILE_DOWN, sizeof(minefield_data));
    memset(minefield_state, TILE_UP, sizeof(minefield_state));
    draw_start_x = FIELD_CENTER_X-((minefield_size_x*16)/2);
    draw_start_y = FIELD_CENTER_Y-((minefield_size_y*16)/2);

    // Generate Mines
    for (int i = 0; i<mine_count; i++) {
        minefield_data[i] = TILE_BOMB;
    }
    shuffle(minefield_data,minefield_size_x*minefield_size_y);

    // Calculate numbers
    int tile = -1;
    for (int y = 0; y < minefield_size_y; y++) {
        for (int x = 0; x < minefield_size_x; x++) {
            if(minefield_data[++tile] == TILE_BOMB) {continue;} // Skip bombs & increment tile

            // check current - 4285-4290
            // don't check current - 4230-4240
            // no bomb_count - 4160-4175

            // Loop through neigbors
            for (int rel_y = y - 1; rel_y < y + 2; rel_y++) {
                for (int rel_x = x - 1; rel_x < x + 2; rel_x++) {
                    // Skip if out of bounds or self
                    if(rel_x == -1 || rel_x == minefield_size_x
                    || rel_y == -1 || rel_y == minefield_size_y) {continue;}
                    if(minefield_data[rel_x+rel_y*minefield_size_x] == TILE_BOMB) {minefield_data[tile]++;}
                }
            }
        }
    }

    drawBoard();
    moveCursor(cursor_x,cursor_y);
}