/*
 * Buttons.c
 *
 * Created: 2017-01-04 21:41:53
 *  Author: Kajetan
 */ 

#include "Buttons.h"
#include "Buzzer.h"

//******** Sta³e ***************************************************************************************

#define T0 ( 20000 / 256 )// ~20ms	podstawa czasu dla debouncingu

//******** Zmienne *************************************************************************************

volatile uint8_t		up_state;		//	Stan przycisku UP		1 - wcisniety
volatile uint8_t		down_state;		//	Stan przycisku DOWN		1 - wcisniety
volatile uint8_t		sel_state;		//	Stan przycisku SEL		1 - wcisniety
uint8_t		repeat_enable;				//	Zezwolenie na autorepetycje
uint16_t	repeat_time;				//	Czestotliwosc autorepetycji ( w klikach na sekunde 1 ~ 50 )
uint16_t	hold_time;					//	Czas przytrzymania do wlaczenia autorepetycji w ms

//******** Procedury ***********************************************************************************

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Inicjuje przyciski
//				
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

void buttons_init ( void ){
	
//	Ustawiamy piny na wejscia z PULL'UP
	
	DDR_UP		&= ~( 1 << PIN_UP );
	DDR_DOWN	&= ~( 1 << PIN_DOWN );
	DDR_POWER	&= ~( 1 << PIN_POWER );
	DDR_SEL		&= ~( 1 << PIN_SEL );
		
	OUT_UP		|=	( 1 << PIN_UP );
	OUT_DOWN	|=	( 1 << PIN_DOWN );
	OUT_POWER	|=	( 1 << PIN_POWER );
	OUT_SEL		|=	( 1 << PIN_SEL );	

	repeat_enable = 0;
	repeat_time = R_TIME( 50 );
	hold_time	= H_TIME( 450 );

}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//	  Procedura wykonujaca autorepetycje i debouncing przyciskow w tle
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

extern	void	on_power_pressed( void );

void	buttons_process ( void ){
		
	static uint8_t cnt = T0;
	static uint16_t up;
	static uint16_t down;
	static uint16_t power;
	static uint16_t sel;
	
//		Odliczamy ~20ms
	
	if ( !( --cnt ) ){
		cnt = T0;
		
		///////////////////////////// DOWN /////////////////////////////

		if ( ! ( IN_DOWN & (1 << PIN_DOWN) ) ){
			
			//	DOWN pressed
			if ( !down ){
				buzzer_on ( BUZ_FREQ_HZ( 2000 ), BUZ_TIME_MS( 25 ) );
				down_state = 1;
			}
			if ( repeat_enable && ( down == hold_time ) ){
				buzzer_on ( BUZ_FREQ_HZ( 2200 ), BUZ_TIME_MS( 10 ) );
				down_state = 1;
			}
			down++;
			
			if ( down > ( hold_time + repeat_time ) )
				down = hold_time;
			
		}else{
			
			//	DOWN released
			
			down = 0;
			down_state = 0;
			
		}

		////////////////////////////// UP //////////////////////////////

		if ( ! ( IN_UP & (1 << PIN_UP) ) ){
		
			//	UP pressed
			if ( !up ){
				buzzer_on ( BUZ_FREQ_HZ( 2000 ), BUZ_TIME_MS( 25 ) );
				up_state = 1;
			}
		
			if ( repeat_enable && ( up == hold_time ) ){
				buzzer_on ( BUZ_FREQ_HZ( 2200 ), BUZ_TIME_MS( 10 ) );
				up_state = 1;
			}
		
			up++;
		
			if ( up > ( hold_time + repeat_time ) )
				up = hold_time;
		
		}else{
		
		//	UP released
		
		up = 0;
		up_state = 0;
		
		}
		
		////////////////////////////// POWER /////////////////////////////
		
		if ( ! ( IN_POWER & (1 << PIN_POWER) ) ){
			
			//	POWER pressed
			if ( !power ){
				buzzer_on ( BUZ_FREQ_HZ( 600 ), BUZ_TIME_MS( 60 ) );
				on_power_pressed();
			}
			
			power = 1;
			
			
			}else{
			
			//	POWER released
			
			power = 0;
			
		}
		
		////////////////////////////// SEL /////////////////////////////

		if ( ! ( IN_SEL & (1 << PIN_SEL) ) ){
			
			//	SEL pressed
			if ( !sel ){
				sel_state = 1;
				buzzer_on ( BUZ_FREQ_HZ( 1000 ), BUZ_TIME_MS( 25 ) );
			}
			
			if ( repeat_enable && ( sel == hold_time ) )			
				sel_state = 1;
				
				sel++;
			
			if ( sel > ( hold_time + repeat_time ) )	
				sel = hold_time;
			
			}else{
			
			//	SEL released
			
			sel = 0;
			sel_state = 0;
			
		}
		
	}
	
}

/*
 * - - - K O N I E C  P L I K U - - -
 *
 * Buttons.c
 *
 * Created: 2017-01-04 21:41:53
 *  Author: Kajetan
 */ 