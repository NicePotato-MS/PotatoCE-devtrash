#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <keypadc.h>
#include <graphx.h>
#include <fontlibc.h>

#include "potatoterm.h"
#include "main.h"

potatoterm_Terminal potatoterm_NewTerminal(int rows, int columns) { // Create new PotatoTerm terminal with properties
    potatoterm_Terminal terminal;
    terminal.rows = rows;
    terminal.columns = columns;
    terminal.cursorX = 0;
    terminal.cursorY = 0;
    terminal.cursorState = POTATOTERM_CURSOR_STATE_ON;

    terminal.characters = (char *)malloc(rows * columns * sizeof(char));
    if (terminal.characters == NULL) { // Failed allocation
        prgmEnd(EXIT_FAILURE);
    }

    terminal.colors = (char *)malloc(rows * columns * sizeof(char));
    if (terminal.colors == NULL) { // Failed allocation
        free(terminal.characters);
        prgmEnd(EXIT_FAILURE);
    }

    return terminal;
}

void potatoterm_FreeTerminal(potatoterm_Terminal *terminal) {  // Free a PotatoTerm terminal from memory
    free(terminal->characters);
    free(terminal->colors);

    // Set pointers to NULL to avoid potential double freeing
    terminal->characters = NULL;
    terminal->colors = NULL;

    free(terminal);
}


