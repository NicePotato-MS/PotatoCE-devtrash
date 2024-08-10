#include <stdint.h>

#include <graphx.h>
#include <keypadc.h>
#include <sys/util.h>

#include "main.h"
#include "misc.h"

void shuffle(unsigned char array[], int size) {
    for (int i = size - 1; i > 0; i--) {
        for (uint8_t a = 0; a < 3; a++) {
            int j = random() % (i + 1);
            // Swap array[i] and array[j]
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
}