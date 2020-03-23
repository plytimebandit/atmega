#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "max7219.h"

/*
 * EEPROM read/write random seed
 *     https://www.avrfreaks.net/comment/558497#comment-558497
 * Generate random:
 *     https://www.avrfreaks.net/comment/205063#comment-205063
 *     https://www.avrfreaks.net/comment/946395#comment-946395
 * avr-libc:
 *     https://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html
 */
void seedTheRand() {
    uint8_t state = eeprom_read_byte((uint8_t *) 0);
    srand(state);
    rand();

    // Alternative: #define RANDINT(n) (random() / (RANDOM_MAX/(n) + 1))
    //          or:                     rand()   / (RAND_MAX  / n  + 1)
    int newState = (int) ((double)rand() / ((double)RAND_MAX + 1) * 255);
    eeprom_write_byte((uint8_t *) 0, newState);
}

int getNextRandom() {
    return (int) ((double)rand() / ((double)RAND_MAX + 1) * 4);
}

int main(void) {
    MAX7219_Init();
    MAX7219_SetBrightness(7);

    seedTheRand();

    for (;;) {
        int randomMatrix = getNextRandom();
        MAX7219_DisplayRowOfMatrix(0, 0b11111111, randomMatrix);

        _delay_ms(1000);

        MAX7219_Clear();

        _delay_ms(200);
    }

    return 0;
}
