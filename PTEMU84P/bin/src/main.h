#ifndef MAIN_HEADER
#define MAIN_HEADER

// GUI config

#define STATUS_BAR_SIZE 16

// Colors

#define COLOR_BG 0x00
#define COLOR_TRANS 0x01
#define COLOR_FG 0x02
#define COLOR_TOPBAR 0x03
#define COLOR_RED 0x04
#define COLOR_BLUE 0x05
#define COLOR_YELLOW 0x06


#define SERIAL_BAUD 115200

#define PSRL_STATE_DISCONNECTED 0
#define PSRL_STATE_POWER 1          // Connected but no serial
#define PSRL_STATE_SERIAL 2         // Connected with serial

#endif