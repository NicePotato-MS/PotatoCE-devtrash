#ifndef KEYBOARD_HEADER
#define KEYBOARD_HEADER

#include <keypadc.h>

#define KEY_GRAPH 0
#define KEY_TRACE 1
#define KEY_ZOOM 2
#define KEY_WINDOW 3
#define KEY_YEQU 4

#define KEY_DEL 7

#define KEY_DOWN 48
#define KEY_LEFT 49
#define KEY_RIGHT 50
#define KEY_UP 51

extern const char *keynames[];

extern int keys[56];

void keyboardLoop();

#endif