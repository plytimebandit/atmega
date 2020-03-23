/*
 * ATMega328P + MAX7219
 *
 * http://ee.cleversoul.com/max7219-source.html
 * https://www.mikrocontroller.net/topic/347500
 * http://www.netzmafia.de/skripten/hardware/RasPi/Projekt-MAX7219/index.html
 *
 * https://github.com/vancegroup-mirrors/avr-libc/blob/master/avr-libc/include/avr/iom328p.h
 */


#include <avr/io.h>
#include "max7219.h"

/*
 **********************************************************
 * ADJUSTABLE CONSTANTS FOR INDIVIDUAL USAGE
 **********************************************************
 */

// number of daisy chained matrixes in total
#define MATRIX_COUNT     4
#define MATRIX_ROWS      8

// used registers and pins
#define ATMEGA_DDR        DDRC
#define ATMEGA_PORT       PORTC
#define DATA_PIN          PC5     // DIN
#define CLK_PIN           PC4     // CLK
#define LOAD_PIN          PC3     // CS

/*
 **********************************************************
 * SET CONSTANTS
 **********************************************************
 */

// registers
#define REG_DECODE        0x09
#define REG_INTENSITY     0x0a
#define REG_SCAN_LIMIT    0x0b
#define REG_SHUTDOWN      0x0c
#define REG_DISPLAY_TEST  0x0f
#define REG_NO_OP         0x00
#define REG_DIGIT0        0x01
#define REG_DIGIT1        0x02
#define REG_DIGIT2        0x03
#define REG_DIGIT3        0x04
#define REG_DIGIT4        0x05
#define REG_DIGIT5        0x06
#define REG_DIGIT6        0x07
#define REG_DIGIT7        0x08

// display intensity
#define INTENSITY_MIN     0x00
#define INTENSITY_MAX     0x0f

// macros
#define DATA_0()      (ATMEGA_PORT &= ~(1 << DATA_PIN))
#define DATA_1()      (ATMEGA_PORT |=  (1 << DATA_PIN))
#define CLK_0()       (ATMEGA_PORT &= ~(1 << CLK_PIN))
#define CLK_1()       (ATMEGA_PORT |=  (1 << CLK_PIN))
#define LOAD_0()      (ATMEGA_PORT &= ~(1 << LOAD_PIN))
#define LOAD_1()      (ATMEGA_PORT |=  (1 << LOAD_PIN))

// private prototypes
static void MAX7219_Write(uint8_t reg_number, uint8_t data);
static void MAX7219_Write_Data(uint8_t reg_number, uint8_t data, int8_t matrix);
static void MAX7219_SendByte(uint8_t data);
static void MAX7219_pinOutput(uint8_t pin);

/*
 **********************************************************
 * SET CONSTANTS
 **********************************************************
 */

#define BUFFER_SIZE     (MATRIX_COUNT * MATRIX_ROWS)
// uint8_t buffer[BUFFER_SIZE];


void MAX7219_Init(void) {
    // set pins as output
    MAX7219_pinOutput(DATA_PIN);
    MAX7219_pinOutput(CLK_PIN);
    MAX7219_pinOutput(LOAD_PIN);

    LOAD_1();
    CLK_0();
    DATA_0();

    MAX7219_Write(REG_SCAN_LIMIT, 7);                   // set up to scan all eight digits
    MAX7219_Write(REG_DECODE, 0);                       // set to "no decode" for all digits
    MAX7219_Write(REG_SHUTDOWN, 1);                     // put MAX7219 into "normal" mode
    MAX7219_Write(REG_DISPLAY_TEST, 0);                 // put MAX7219 into "normal" mode
    MAX7219_Clear();                                    // clear all digits
    MAX7219_SetBrightness(INTENSITY_MAX);               // set to maximum intensity
}

void MAX7219_pinOutput(uint8_t pin) {
    ATMEGA_DDR |= (1 << pin);
}

void MAX7219_Clear(void) {
    // turn all segments off
    /*for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 0;
    }*/
    //for (uint8_t i = 0; i < MATRIX_COUNT; i++) {
        for (uint8_t j = 0+1; j <= MATRIX_ROWS; j++) {
            MAX7219_Write(j, 0);
        }
    //}
}

void MAX7219_SetBrightness(uint8_t brightness) {
    // mask off extra bits and set brightness
    brightness &= INTENSITY_MAX;
    for (uint8_t i = 0; i < MATRIX_COUNT; i++) {
        MAX7219_Write(REG_INTENSITY, brightness);
    }
}

/*
 * Use this function to write data if
 *   - there is just one matrix or
 *   - all daisy chained matrixes shall display the same thing.
 *
 * row: 1st row is 0, last row is 7
 * cols: one byte e.g. 0b10101010
 */
void MAX7219_DisplayRow(uint8_t row, uint8_t cols) {
    MAX7219_DisplayRowOfMatrix(row, cols, -1);
}

/*
 * Use this function to write data if you want to send data to one specific matrix out of several.
 *
 * row: 1st row is 0, last row is 7
 * cols: one byte e.g. 0b10101010
 * matrix: 0-n, where 0 is the 1st matrix and n is the max number of matrixes (MATRIX_COUNT) - 1; for the meaning of the special value -1 see MAX7219_DisplayRow().
 */
void MAX7219_DisplayRowOfMatrix(uint8_t row, uint8_t cols, int8_t matrix) {
    if (row < 0 && row > 7) {
        return;
    }
    if (matrix >= MATRIX_COUNT) {
        matrix = MATRIX_COUNT - 1;
    }
    MAX7219_Write_Data(row, cols, matrix);
}

static void MAX7219_Write_Data(uint8_t row, uint8_t dataout, int8_t matrix) {
    // take LOAD low to begin
    LOAD_0();

    uint8_t reg_number = row + 1;

    if (matrix < 0) {
        // write register number to MAX7219 and write data to MAX7219
        // also save data in buffer
        /*for (uint8_t i = 0; i <= MATRIX_COUNT; i++) {
            uint8_t bufferPos = row + i * MATRIX_ROWS;
            buffer[bufferPos] = dataout;
        }*/
        for (uint8_t i = 0; i < MATRIX_COUNT; i++) {
            MAX7219_SendByte(reg_number);
            MAX7219_SendByte(dataout);
	}
    } else {
        // same as -1 but change only byte of specified matrix
        for (int8_t i = (MATRIX_COUNT-1); i >= 0; i--) {
            // uint8_t bufferPos = row + i * MATRIX_ROWS;

            if (i == matrix) {
                // buffer[bufferPos] = dataout;
                MAX7219_SendByte(reg_number);
                MAX7219_SendByte(dataout);
            } else {
                // MAX7219_SendByte(buffer[bufferPos]);
                MAX7219_SendByte(REG_NO_OP);
                MAX7219_SendByte(REG_NO_OP);
            }
        }
    }

    // take LOAD high to latch in data and end writing
    LOAD_1();
}

static void MAX7219_Write(uint8_t reg_number, uint8_t dataout) {
    // take LOAD low to begin
    LOAD_0();

    // write register number to MAX7219 and write data to MAX7219
    MAX7219_SendByte(reg_number);
    MAX7219_SendByte(dataout);

    // take LOAD high to latch in data and end writing
    LOAD_1();
}

static void MAX7219_SendByte(uint8_t dataout) {
    for (uint8_t i = MATRIX_ROWS; i > 0; i--) {
        CLK_0();
        // output one data bit
        uint8_t mask = 1 << (i - 1);
        if (dataout & mask) {
            DATA_1();
        } else {
            DATA_0();
        }
        CLK_1();
    }
}
