/*
 * Final Attiny85-74hc595.c
 *Attiny85	74hc595
 *pin5	-	pin11	PINB0	SHIFT
 *pin3	-	pin12	PINB4	LATCH
 *pin7	-	pin14	PINB2	DATA		
 *	
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define SPEED_STEP_DELAY 2                             /* milliseconds */

//#define RAND_MAX 255;

uint8_t sendbyte = 0x00;			//Value to send to 74hc595

void latch74hc595(void){
	PORTB |= (1 << PINB4);
	_delay_ms(1);
	PORTB &= ~(1 << PINB4);
}

void shiftin74hc595(uint8_t bit8value){

	PORTB =	0;
	int n = 7;
	
	while(n >= 0){
	
		if (bit8value & (1 << (7-n))){
			PORTB |= (1 << PINB2);
		}
		
		else{
			PORTB &= ~(1 << PINB2);
		}
		
		_delay_ms(1);
		PORTB |= (1 << PINB0);
		_delay_ms(1);
		PORTB &= ~(1 << PINB0);
		n--;
	}
	
}

static inline void initADC0(void){
	ADMUX |= (1 << MUX0)| (1 << MUX1);
	ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
	ADCSRA |= (1 << ADEN);
}

static inline void initTimer0(void) {
  TCCR0A |= (1 << WGM00);                             /* Fast PWM mode */
  TCCR0A |= (1 << WGM01);                       /* Fast PWM mode, pt.2 */
  TCCR0A |= (1 << COM0B1);                        /* output PWM to pin */
  TCCR0B |= (1 << CS02);                 /* Clock with /1024 prescaler */
  //TCCR0B |= (1 << CS00);         /* Clock with /1024 prescaler, pt.2 */
}

int main(void)
{
	//Configure pins
	DDRB |= (1 << PINB0)|(1 << PINB4)|(1 << PINB2)|(1 << PINB1);
	PORTB &= ~(1 << PINB0);
	uint8_t ledValue;
	uint16_t adcValue;
	uint8_t i;
	uint16_t updateSpeed;
	
	initADC0();
	initTimer0();
	OCR0B = 0;
	
    while (1) 
    {
	
		ADCSRA |= (1 << ADSC);
		loop_until_bit_is_clear(ADCSRA, ADSC);
		adcValue = ADC;
		ledValue = (adcValue >> 7); 
		
		sendbyte = 0;
		updateSpeed = (adcValue >> 2);
		for(i = 0; i <= ledValue; i++){
			sendbyte |= (1 << i);
		}
		
		if (OCR0B < updateSpeed) {
			while (OCR0B < updateSpeed) {
				OCR0B++;
				_delay_ms(SPEED_STEP_DELAY);
			}
		}
		else {
			while (OCR0B > updateSpeed) {
				OCR0B--;
				_delay_ms(SPEED_STEP_DELAY);
			}
		}
		
		shiftin74hc595(sendbyte);
		_delay_ms(1);
		latch74hc595();
		_delay_ms(1);
    }
}
