/*
 * Driver.c
 *		( modu³ wykonawczy )
 * Created: 2017-01-04 21:41:53
 *  Author: Kajetan
 */ 

#include "Driver.h"
#include <avr/interrupt.h>

//******** Zmienne *************************************************************************************

uint8_t		g_temp_ok = 0;		//	temp. zadana osiagnieta
uint8_t		g_heat_enable = 0;	//	wlaczenie grzania

uint16_t	g_pump_time_off = 0;	//	krok 25ms
uint16_t	g_pump_time_on = 0;		//	krok 25ms ; =0 - OFF ; >90*40 - ON
uint16_t	g_pump_time_off_set;	//	Czas ON podczas trawienia, krok 25ms
uint16_t	g_pump_time_on_set;	//	Czas OFF podczas trawienia, krok 25ms, Jesli 0 wtedy pompa ON

uint16_t	g_curr_temp = 0;		// akt. temp. z czujnika
uint16_t	g_hysteresis;			// histereza
uint16_t	g_set_temp;				// temp. zadana
int16_t		g_hysteresis_set;

//******** Makrodefinicje ******************************************************************************

#define	PUMP_ON() OUT_DRP &= ~(1<<PIN_DRP)
#define	PUMP_OFF() OUT_DRP |= (1<<PIN_DRP)
#define	HEAT_ON() OUT_DRH &= ~(1<<PIN_DRH)
#define	HEAT_OFF() OUT_DRH |= (1<<PIN_DRH)

//******** Procedury ***********************************************************************************

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//					Procedury zmieniajace zbocze
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

inline	void	set_falling_edge ( void ){
	
	MCUCR	|= (1<<ISC01);
	MCUCR	&= (1<<ISC00);
	
}

inline	void	set_rising_edge ( void ){
	
	MCUCR	|= (1<<ISC01);
	MCUCR	|= (1<<ISC00);
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//					Inicjuje zewnetrzny uklad wykonawczy
//				
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

void	driver_init ( void ){
	
//	Ustawiamy kierunek pinow :

	DDR_DRH |= (1<<PIN_DRH);
	DDR_DRP |= (1<<PIN_DRP);
	DDR_DRZ &= ~(1<<PIN_DRZ);	//Detekcja zera na wejscie

//	Wylaczamy pompe i grzalke

	PUMP_OFF();
	HEAT_OFF();
	
//	Wlaczamy przerwanie 0 na zbocze opadajace :

	GICR	|= (1<<INT0);
	set_falling_edge();
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Procedura obslugi przerwania INT0 ( przelaczanie w zerze )
//								T ~ 25ms
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

ISR( INT0_vect ){
	
	static uint8_t edge = 0;	//Aktualne zbocze, 0 - opadajace, 1 - narastajace
	static uint16_t cnt = 0;	//licznik 25ms
	
	if ( edge ){	//Zmieniamy zbocze na przeciwne :
	
		edge = 0;
		set_falling_edge();
		
	}else{
		
		edge = 1;
		set_rising_edge();
	}
	
	//	praca pompy :
	
	if( !g_pump_time_on ){
			
		PUMP_OFF();
			
	}else if ( g_pump_time_on > (90 * 40) ){
			
		PUMP_ON();
			
	}else{
			
		if ( !cnt )
			PUMP_ON();
		else if ( cnt == g_pump_time_on )
			PUMP_OFF();
		else if ( cnt > (g_pump_time_on + g_pump_time_off) )
			cnt = -1;
				
		cnt++;
	}
	
	//	praca grzalki
	
	if ( g_heat_enable ){
	
		if ( g_curr_temp < ( g_set_temp - g_hysteresis ) )
			HEAT_ON();
		
		else if ( g_curr_temp > ( g_set_temp + g_hysteresis ) )
			HEAT_OFF();
			
	}
	else{
	
		HEAT_OFF();
	
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