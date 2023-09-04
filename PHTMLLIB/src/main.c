#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ti/screen.h>
#include <ti/getcsc.h>
#include <graphx.h>
#include <fontlibc.h>

void parseHTML(const char *htmlString) {
    
}

void renderPHTML(const char *phtml, unsigned int x, unsigned int y) {

}

/* Main function, called first */
int main(void)
{
    gfx_Begin();
    gfx_ZeroScreen();
    

    /* Waits for a key */
    while (!os_GetCSC());

    /* Return 0 for success */
    return 0;
}