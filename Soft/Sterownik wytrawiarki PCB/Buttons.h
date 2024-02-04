/*
 * Buttons.h
 *
 * Created: 2017-01-04 21:42:58
 *  Author: Kajetan
 */ 


#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <avr/io.h>

//******** Definicje pinow *****************************************************************************

#define	PIN_UP		PORTD7
#define	PIN_DOWN	PORTB0
#define	PIN_POWER	PORTD5
#define	PIN_SEL		PORTD6

//		Rejestr wyjsciowy ( PORT ) :

#define	OUT_UP		PORTD
#define	OUT_DOWN	PORTB
#define	OUT_POWER	PORTD
#define	OUT_SEL		PORTD

//		Rejestr kierunku ( DDR ) :

#define	DDR_UP		*( &OUT_UP - 1 )
#define	DDR_DOWN	*( &OUT_DOWN - 1 )
#define	DDR_POWER	*( &OUT_POWER - 1 )
#define	DDR_SEL		*( &OUT_SEL - 1 )

//		Rejestr wejsciowy ( PIN ) :

#define	IN_UP		 *( &OUT_UP - 2 )
#define	IN_DOWN		 *( &OUT_DOWN - 2 )
#define	IN_POWER	 *( &OUT_POWER - 2 )
#define	IN_SEL		 *( &OUT_SEL - 2 )

//******** Procedury ***********************************************************************************

void	buttons_init		( void );
void	buttons_process		( void );

//******** Makrodefinicje ******************************************************************************

#define H_TIME(x)	(uint16_t) ( (x) / 20 )				//Makro dla ustawiania zmiennej hold_time
#define R_TIME(x)	(uint16_t) ( 1.0 / ( (x) * 0.02 ) )	//Makro dla ustawiania zmiennej repeat_time

//******** Zmienne *************************************************************************************

extern	volatile uint8_t		up_state;		//	Stan przycisku UP		1 - wcisniety
extern	volatile uint8_t		down_state;		//	Stan przycisku DOWN		1 - wcisniety
extern	volatile uint8_t		sel_state;		//	Stan przycisku SEL		1 - wcisniety
extern	uint8_t		repeat_enable;			//	Zezwolenie na autorepetycje
extern	uint16_t	repeat_time;			//	Czestotliwosc autorepetycji ( w klikach na sekunde 1 ~ 50 )
extern	uint16_t	hold_time;				//	Czas przytrzymania do wlaczenia autorepetycji w ms

#endif /* BUTTONS_H_ */

/*
 * - - - K O N I E C  P L I K U - - -
 *
 * Buttons.h
 *
 * Created: 2017-01-04 21:42:58
 *  Author: Kajetan
 */ 