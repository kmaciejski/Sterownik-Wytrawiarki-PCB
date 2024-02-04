/*
 * Buzzer.c
 *
 * Created: 2016-04-24 19:18:01
 *  Author: Kajetan
 */ 

#include "Buzzer.h"


//******** Zmienne *************************************************************************************

volatile	uint16_t	buz_time = 0;

//******** Procedury ***********************************************************************************

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Inicjuje buzzer do pracy tzn. ustawia odpowiednie piny
//		i rejestry
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	buzzer_init ( void ){
	
//		Pin buzzera na wyjscie	
	
	DDR_BUZ	|= ( 1 << PIN_BUZ );
	
//		Ustawiamy TIMER 1 do pracy w trybie CTC
	
	TCCR1A |= (1<<COM1B0);
	TCCR1B |= (1<<WGM12) | (1<<CS11);

//		Dzwiek 1000Hz przez 1s
	
	buzzer_on ( BUZ_FREQ_HZ( 1000 ), BUZ_TIME_MS( 1000 ) );
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Wylacza buzzer po okreslonym czasie
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	buzzer_process ( void ){
	
	if( buz_time ){
		
		if( ! ( --buz_time ) ){
			
			buzzer_off();
		
		}
		
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Wylacza buzzer
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	buzzer_off ( void ){
	
//		Odlaczamy pin od CTC
	
	TCCR1A &= ~(1<<COM1B0);	
	
//		Pin buzzera na stan niski
	
	OUT_BUZ	&= ~( 1 << PIN_BUZ );
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Wlacza buzzer f - wartosc rej. OCR1A, t - czas w 0.256ms
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	buzzer_on ( uint16_t f, uint16_t t ){
	
//		Zerujemy licznik

	TCNT1 = 0;

//		Ustawiamy rej. OCR1A

	OCR1A = f;

//		Ustawiamy czas buzzera

	buz_time = t;
				
//		Podlaczamy buzzer do CTC

	TCCR1A |= (1<<COM1B0);	

}

/*
 * - - - K O N I E C  P L I K U - - -
 *
 * Buzzer.c
 *
 * Created: 2016-04-24 19:18:01
 *  Author: Kajetan
 */ 