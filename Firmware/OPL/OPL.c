#include "OPL.h"

#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */

#define YM_WR  4
#define YM_A0  3
#define YM_IC  5

void opl_init() {
    DDRC = (1 << YM_A0) | (1 << YM_WR) | (1 << YM_IC);
    PORTC = (1 << YM_WR) | (1 << YM_IC) | (0 << YM_A0);
    
    opl_reset();
}

void opl_reset() {
    PORTC &= ~(1 << YM_IC);
    _delay_ms(1);
    PORTC |= (1 << YM_IC);
}

void opl_write(unsigned char address, unsigned char data) {
    PORTC &= ~(1 << YM_A0);
    PORTD = address;
    PORTC &= ~(1 << YM_WR);
    _delay_us(1);
    PORTC |= (1 << YM_WR);
    _delay_us(4);
    
    PORTC |= (1 << YM_A0);
    PORTD = data;
    PORTC &= ~(1 << YM_WR);
    _delay_us(1);
    PORTC |= (1 << YM_WR);
    _delay_us(23);
}