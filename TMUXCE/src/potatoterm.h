#ifndef HEADER_POTATOTERM
#define HEADER_POTATOTERM

#define POTATOTERM_MAX_ROWS 53
#define POTATOTERM_MAX_COLS 24

#define POTATOTERM_FNTX_SIZE 6
#define POTATOTERM_FNTY_SIZE 10

#define POTATOTERM_MAX_X MAX_ROWS*X_SIZE
#define POTATOTERM_MAX_Y MAX_COLS*Y_SIZE

#define POTATOTERM_CURSOR_STATE_OFF 0
#define POTATOTERM_CURSOR_STATE_ON 1

typedef struct {
    int rows;
    int columns;
    int cursorX;
    int cursorY;
    int cursorState;
    char *characters;
    char *colors;
} potatoterm_Terminal;

potatoterm_Terminal potatoterm_NewTerminal(int rows, int columns); // Create new PotatoTerm terminal with properties
void potatoterm_FreeTerminal(potatoterm_Terminal *terminal); // Free a PotatoTerm terminal from memory

#endif