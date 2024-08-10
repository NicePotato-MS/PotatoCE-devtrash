#ifndef POTATOGFX_HEADER
#define POTATOGFX_HEADER

#include <stdint.h>

#include <graphx.h>

void renderTile(unsigned char *sprite, uint8_t location[]);
void displayTile(unsigned x,unsigned y);
unsigned int tileVRAMX(unsigned int x);
unsigned int tileVRAMY(unsigned int y);
void moveCursor(unsigned int x, unsigned int y);
void initBoard();

extern unsigned char* sprites[];

extern unsigned char minefield_data;
extern unsigned char minefield_state;
extern int minefield_size_x;
extern int minefield_size_y;
extern int mine_count;
extern int cursor_x;
extern int cursor_y;
extern clock_t cursor_lastMove;
extern bool cursorHasMoved;
extern int lastCursorMove;
extern int seed;
extern clock_t game_start;

#define FIELD_MAX_X 17
#define FIELD_MAX_Y 15
#define FIELD_CENTER_X FIELD_MAX_X*8
#define FIELD_CENTER_Y FIELD_MAX_Y*8

#define TILE_DOWN 0
#define TILE_1 1
#define TILE_2 2
#define TILE_3 3
#define TILE_4 4
#define TILE_5 5 
#define TILE_6 6
#define TILE_7 7
#define TILE_8 8
#define TILE_UP 9
#define TILE_FLAG 10
#define TILE_FLAGERROR 11
#define TILE_QUESTION 12
#define TILE_QUESTIONDOWN 13
#define TILE_BOMB 14
#define TILE_HITBOMB 15
#define TILE_NOBOMB 16

#endif