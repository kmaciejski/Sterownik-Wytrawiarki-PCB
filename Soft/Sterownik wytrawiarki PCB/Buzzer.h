/*
 * Buzzer.h
 *
 * Created: 2016-04-24 19:18:18
 *  Author: Kajetan
 */ 


#ifndef BUZZER_H_
#define BUZZER_H_

#include <avr/io.h>

//******** Definicje pinow *****************************************************************************

#define	PIN_BUZ		 PORTB2

//		Rejestr wyjsciowy ( PORT ) :

#define	OUT_BUZ		 PORTB

//		Rejestr kierunku ( DDR ) :

#define	DDR_BUZ		 *( &OUT_BUZ - 1 )

//		Rejestr wejsciowy ( PIN ) :

#define	IN_BUZ		 *( &OUT_BUZ - 2 )

//******** Procedury ***********************************************************************************

void	buzzer_off		( void );
void	buzzer_on		( uint16_t f, uint16_t t );
void	buzzer_init		( void );
void	buzzer_process	( void );

//******** Makrodefinicje *****************************************************************************

#define BUZ_FREQ_HZ(f)	( ( 500000 - f ) / f )
#define BUZ_TIME_MS(t)	(uint16_t)( ( (uint32_t)t * 1000 ) / 256 )

#endif /* BUZZER_H_ */

/*
 * - - - K O N I E C  P L I K U - - -
 *
 * Buzzer.h
 *
 * Created: 2016-04-24 19:18:18
 *  Author: Kajetan
 */ 