#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/timers.h>

#include <keypadc.h>
#include <graphx.h>
#include <fontlibc.h>
#include <srldrvce.h>

#define gfx_vramend 0xD52C00
#define gfx_vramsize 76800

void scrollScreenUp(unsigned int amount) {
    unsigned int offset = GFX_LCD_WIDTH*amount;
    memcpy(gfx_vram, gfx_vram+offset, gfx_vramsize-offset);
}

/* Main function, called first */
int main(void)
{
    gfx_Begin();
    gfx_ZeroScreen();
    gfx_FillScreen(gfx_white);

    /* Waits for a key */
    do {
        kb_Scan();

    } while (!kb_IsDown(kb_KeyClear));

    /* Return 0 for success */
    gfx_End();
    return 0;
}