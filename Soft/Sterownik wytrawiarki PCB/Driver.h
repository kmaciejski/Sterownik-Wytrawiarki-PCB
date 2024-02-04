/*
 * Driver.h
 *		( modu³ wykonawczy )
 * Created: 2017-01-04 21:42:58
 *  Author: Kajetan
 */ 


#ifndef DRIVER_H_
#define DRIVER_H_

#include <avr/io.h>

//******** Definicje pinow *****************************************************************************

#define	PIN_DRH		PORTD4	//DR - Driver, H - Heater control
#define	PIN_DRP		PORTB6	//DR - Driver, P - Pump control
#define	PIN_DRZ		PORTD2	//DR - Driver, Z - Zero detect signal

//		Rejestr wyjsciowy ( PORT ) :

#define	OUT_DRH		PORTD
#define	OUT_DRP		PORTB
#define	OUT_DRZ		PORTD

//		Rejestr kierunku ( DDR ) :

#define	DDR_DRH	*( &OUT_DRH - 1 )
#define	DDR_DRP	*( &OUT_DRP - 1 )
#define	DDR_DRZ	*( &OUT_DRZ - 1 )

//		Rejestr wejsciowy ( PIN ) :

#define	IN_DRH	*( &OUT_DRH - 2 )
#define	IN_DRP	*( &OUT_DRP - 2 )
#define	IN_DRZ	*( &OUT_DRZ - 2 )

//******** Zmienne *************************************************************************************

extern uint8_t	g_temp_ok;		//	temp. zadana osiagnieta
extern uint8_t	g_heat_enable;	//	wlaczenie grzania

extern uint16_t	g_pump_time_off;	//	krok 25ms
extern uint16_t	g_pump_time_on;		//	krok 25ms
extern uint16_t	g_pump_time_off_set;	//	Czas ON podczas trawienia, krok 25ms
extern uint16_t	g_pump_time_on_set;		//	Czas OFF podczas trawienia, 	krok 25ms

extern uint16_t	g_curr_temp;		//	akt. temp. z czujnika
extern uint16_t	g_hysteresis;		//	histereza
extern uint16_t	g_set_temp;		//	temp. zadana
extern int16_t	g_hysteresis_set;//	histereza podczas trawienia

//******** Procedury ***********************************************************************************

void	driver_init		( void );

inline	void	heat_enable		( void ){ g_heat_enable = 1; }
inline	void	heat_disable	( void ){ g_heat_enable = 0; }

//******** Makrodefinicje ******************************************************************************

#endif /* DRIVER_H_ */

/*
 * - - - K O N I E C  P L I K U - - -
 *
 * Driver.h
 *
 * Created: 2017-01-04 21:42:58
 *  Author: Kajetan
 */ 