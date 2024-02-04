/*
 * main.c
 *
 * Created: 2016-04-20 21:00:21
 * Author : Kajetan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/eeprom.h> 
#include "HD44780.h"
#include "Buzzer.h"
#include "Buttons.h"
#include "ds18b20.h"
#include "Driver.h"

//******** Makrodefinicje ******************************************************************************

#define soft_reset()        \
do                          \
{                           \
	wdt_enable(WDTO_15MS);  \
	for(;;)                 \
	{                       \
	}                       \
} while(0)

//************* Definicje ******************************************************************************

//Punkty pracy programu :

#define MENU_MAIN			4
#define	MENU_ETCHING		5
#define	MENU_ETCHING_END	6
#define	MENU_HEAT_START		7	//Rozgrzewanie roztworu
#define	MENU_PARAMS			8	//Nastawianie parametrow

#define	MENU_TIME_SET		0	//czas trawienia
#define	MENU_TEMP_SET		1	//temp trawienia
#define	MENU_PUMP_SET		2	//interwaly pompy
#define	MENU_HYSTERESIS		3	//histereza

//Parametry dla f-cji print_time :

#define SEL_NONE		0
#define	SEL_MIN			1
#define	SEL_SEC			2

//Parametry dla f-cji print_temperature :

#define SEL_NONE		0
#define	SEL_ONES		1	// Jednosci
#define	SEL_TENS		2	// cz. dziesietne
#define	SEL_ALL			3	// cala liczba

//Parametry dla f-cji m_pump_set :

#define SEL_T_ON		1	// Czas wlaczenia pompy ( praca )
#define	SEL_T_OFF		2	// Czas wylaczenia pompy ( stop )

//********* Procedury - deklaracje *********************************************************************

//		Operujace na pamieci

void		memzero		( uint8_t *dest, uint8_t size );
void		memfill		( uint8_t *dest, uint8_t fill, uint8_t size );
uint8_t		memcmp		( uint8_t *src, uint8_t *dest, uint8_t size );
void		memcpy		( uint8_t *src, uint8_t *dest, uint8_t size );

uint8_t		putdec16		( uint16_t b );
void		print_temperature ( int16_t temp, uint8_t sel );
void		print_time ( uint8_t mm, uint8_t ss, uint8_t sel, uint8_t zero_print );

void		timer0_init	( void );
void		timer2_init	( void );

//Akcje :

void		countdown_process ( void );
void		on_end_countdown ( void );

//Menu :

void		m_main ( void );
void		m_etching( void );
void		m_etching_end( void );
void		m_heat_start( void );
void		m_params( void );

void		m_time_set( void );
void		m_temp_set( void );
void		m_pump_set( void );
void		m_hysteresis( void );

//********* Zmienne globalne **************************************************************************

volatile	uint8_t		power_on = 0;

volatile	int16_t		temp_set;
volatile	int16_t		temp_curr = 250;

volatile	uint16_t	time_mm_set;	//Czas trawienia ( minuty )
volatile	uint16_t	time_ss_set;	//Czas trawienia ( sekundy )
volatile	uint16_t	time_mm;	//Pozostaly czas trawienia ( minuty )
volatile	uint16_t	time_ss;	//Pozostaly czas trawienia ( sekundy )

volatile	uint8_t		time_update = 0;	//Flaga sygnalizujaca zmiane czasu
volatile	uint8_t		temp_update = 0;	//Flaga sygnalizujaca zmiane temperatury
volatile	uint8_t		temp_update2 = 0;	//Flaga sygnalizujaca zmiane temperatury

volatile	uint8_t		countdown_enable	= 0;	//flaga wlaczajaca odliczanie
volatile	uint8_t		temp_read_enable	= 0;	//flaga wlaczajaca odczyt temperatury

volatile	uint8_t		menu = MENU_MAIN;
volatile	uint8_t		menu_init = 1;

//********* Zmienne EEPROM ****************************************************************************

uint16_t	EEMEM	eep_pump_time_off	= 480;	// Czas ON podczas trawienia, krok 25ms
uint16_t	EEMEM	eep_pump_time_on	= 80;	// Czas OFF podczas trawienia, krok 25ms
uint16_t	EEMEM	eep_hysteresis		= 5;	// histereza
uint16_t	EEMEM	eep_set_temp		= 450;	// temp. zadana
uint16_t	EEMEM	eep_time_mm			= 15;	//czas trawienia ( minuty )
uint16_t	EEMEM	eep_time_ss			= 0;	//czas trawienia ( sekundy )

char *tab_parameter_names[] =
{
	"Czas trawienia",
	"Temp trawienia",
	"Pompa praca",
	"Histereza",
	"Powrot \x7f"
};

typedef void( * PVOID )( void );

PVOID tab_menu_func[] =
{
	
	m_time_set,
	m_temp_set,
	m_pump_set,
	m_hysteresis,
	m_main,
	m_etching,
	m_etching_end,
	m_heat_start,
	m_params
	
};

//********* Punkt wejscia programu ********************************************************************

int		main ( void )
{
    
	//		Uklad wykonawczy
	
	driver_init();
	
	//		Kod inicjujacy przyciski i sheduler :

	timer0_init();
	buttons_init();
	
	//		Wlaczamy przerwania :

	sei();
	
	while ( !power_on );	//Czekamy na wcisniecie przycisku zasilania
	
	timer2_init();
	LCD_init();
	buzzer_init();
	
	LCD_Clear();
	
    while ( 69 )
    {
		
		if ( temp_read_enable ){
			
			uint16_t tmp;
			tmp = ds18b20_gettemp();
			
			if ( ( tmp != DS18B20_NO_SENSOR )  && ( tmp != DS18B20_CRC_ERR ) )
				g_curr_temp  = temp_curr = tmp;
					
			temp_update2 = temp_update = 1;
		}
		
		//Wywolujemy odpowiedni interfejs :
		
		tab_menu_func [ menu ] ();
		
	//END OF WHILE
		
	}
	
//END OF MAIN
	
}

//********* Procedury - definicje **********************************************************************

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Menu konca trawienia
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_etching_end ( void ){
	
	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
		

	}
	
	//Proces menu :

	
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Menu trawienia
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_etching ( void ){
	
	if( menu_init ){
		menu_init = 0;
	//Inicjalizacja menu :	
		
		time_mm = time_mm_set;
		time_ss = time_ss_set;
	
		countdown_enable = 1;	//Wlaczamy potrzebne procesy
		temp_read_enable = 1;

		g_pump_time_on = g_pump_time_on_set;
		g_pump_time_off = g_pump_time_off_set;
				
		LCD_Clear();			//Ekran menu
		LCD_GoTo(0,0);
		LCD_WriteText("Trawienie ------");
		LCD_GoTo(0,1);
		LCD_WriteText("Pozostalo --:--");
		
	}
	
	//Proces menu :
	
	if ( time_update ){
		time_update = 0;
		
		LCD_GoTo( 0,1 );
		LCD_WriteText("Pozostalo ");
		print_time( time_mm, time_ss, SEL_NONE, 1 );
		
	}
	if ( temp_update ){
		temp_update = 0;
		
		LCD_GoTo( 0,0 );
		LCD_WriteText("Trawienie ");
		print_temperature( temp_curr, SEL_NONE );
		
	}
	
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Menu glowne
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_main ( void ){
	
	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
		
		countdown_enable = 0;	//Wylaczamy niepotrzebne procesy
		temp_read_enable = 0;

		heat_disable();
		g_pump_time_on = 0;
		
		LCD_Clear();			//Ekran menu
		LCD_GoTo(0,0);
		LCD_WriteText("SEL: trawienie");
		LCD_GoTo(0,1);
		LCD_WriteText("UP/DOWN: nastawy");
		
	}
	
	//Proces menu :

	if ( sel_state ){
		sel_state = 0;
		
		menu = MENU_HEAT_START;
		menu_init = 1;	
		
	}else if ( up_state || down_state ){	
		up_state = 0;
		down_state = 0;
		
		menu = MENU_PARAMS;
		menu_init = 1;	
		
	}
	
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Menu wyboru nastaw
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_params ( void ){
	
	static	uint8_t menu_pos;
	
	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
		menu_pos = 0;
		LCD_Clear();
		
	}
	
	//Proces menu :
	
	LCD_GoTo(0,0);
	LCD_WriteText("Zmiana nastaw :");
	
	LCD_GoTo(0,1);
	putdec16( menu_pos + 1 );
	LCD_WriteData('.');
	LCD_WriteText( tab_parameter_names[menu_pos] );
	
	if ( sel_state ){
		sel_state = 0;
		
		if( menu_pos == 4 )
			menu = MENU_MAIN;
		else
			menu = menu_pos;
			
		menu_init = 1;

	}
	if ( up_state ){
		up_state = 0;
		
		menu_pos ++;
		if ( menu_pos == 5 )
			menu_pos = 0;	
		LCD_Clear();
		
	}
	if ( down_state ){
		down_state = 0;
		
		if ( !menu_pos )
		menu_pos = 5;
		menu_pos--;
					
		LCD_Clear();
		
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//						Ustawianie czasu trawienia
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_time_set( void ){
	
	static uint8_t mm;
	static uint8_t ss;
	static uint8_t sel;
	
	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
		
		LCD_Clear();
		mm = time_mm_set;
		ss = time_ss_set;
		sel = SEL_MIN;
		repeat_enable = 1;		//Wlaczamy autorepetycje klawiszy
		
	}
	
	//Proces menu :
	
	LCD_GoTo(1,0);
	LCD_WriteText("Czas trawienia");
	
	LCD_GoTo(5,1);
	print_time( mm, ss, sel, 0 );
	
	if ( sel_state ){
		sel_state = 0;
		
		if ( sel == SEL_SEC ){
			
			eeprom_write_word ( &eep_time_ss, time_ss_set = ss);
			eeprom_write_word ( &eep_time_mm, time_mm_set = mm);
			menu = MENU_PARAMS;
			menu_init = 1;
			repeat_enable = 0;	//Wylaczamy autorepetycje klawiszy
		
		}
		
		sel++;

	}
	if ( up_state ){	//Próg gorny 99 min
		up_state = 0;
		
		if ( sel == SEL_MIN ){
			
			mm++;
			if ( mm > 99 )
				mm = 0;
			
		}else if ( sel == SEL_SEC ){
		
			ss++;
			if ( ss > 59 )
				ss = 0;
		
		}
			
		LCD_Clear();
		
	}
	if ( down_state ){	//Próg dolny 1 min
		down_state = 0;
		
		if ( sel == SEL_MIN ){
			
			mm--;
			
			if ( mm > 99 )
				mm = 99;
			
		}else if ( sel == SEL_SEC ){
			
			if ( !ss )
				ss = 60;
			ss--;
			
		}
		
		LCD_Clear();
		
	}
		
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//					Ustawianie temperatury trawienia
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_temp_set( void ){
	
	static uint16_t temp;
	static uint8_t sel;
	
	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
		
		LCD_Clear();
		temp = temp_set;
		sel = SEL_ONES;
		repeat_enable = 1;		//Wlaczamy autorepetycje klawiszy
		
	}
	
	//Proces menu :
	
	LCD_GoTo(1,0);
	LCD_WriteText("Temp trawienia");
	
	LCD_GoTo(4,1);
	print_temperature( temp, sel );
	
	if ( sel_state ){
		sel_state = 0;
		
		if ( sel == SEL_TENS ){
			
			eeprom_write_word ( &eep_set_temp, temp_set = temp);
			menu = MENU_PARAMS;
			menu_init = 1;
			repeat_enable = 0;	//Wylaczamy autorepetycje klawiszy
			
		}

		sel++;

	}
	if ( up_state ){	//Próg gorny 75.0 C
		up_state = 0;
		
		if ( sel == SEL_ONES ){
			
			if ( temp < 750  )
				temp += 10;
			
		}else if ( sel == SEL_TENS ){
			
			temp += 1;
			
			if ( ! (temp % 10) )	//Zero w cz. dziecietnych
				temp -= 10;
			
		}
		
		LCD_Clear();
		
	}
	if ( down_state ){	//Próg dolny 25.0 C
		down_state = 0;
		
		if ( sel == SEL_ONES ){
			
			if ( temp > 250  )
				temp -= 10;
			
		}else if ( sel == SEL_TENS ){
			
			if ( ! (temp % 10) )	//Zero w cz. dziecietnych
				temp += 10;
			
			temp --;
			
		}
		
		LCD_Clear();
		
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//					Ustawianie interwalow czasowych pompy
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_pump_set( void ){

	static uint16_t t_on;
	static uint16_t t_off;
	uint16_t t_x;
	static uint8_t sel;

	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
	
		LCD_Clear();
	
		t_on = g_pump_time_on_set;
		t_off = g_pump_time_off_set;
		
		sel = SEL_T_ON;
		repeat_enable = 1;		//Wlaczamy autorepetycje klawiszy
	
	}

//Proces menu :

					//Wyswietlanie nastaw
	LCD_GoTo(0,0);
	
	if ( sel == SEL_T_ON )
		LCD_WriteText("Czas pracy pompy");
	else
		LCD_WriteText("  Czas przerwy");
	
	LCD_GoTo(3,1);
	LCD_WriteData('>');
	
	if ( sel == SEL_T_ON )
		t_x = t_on;
	else
		t_x = t_off;
		
	if ( !t_x )
		LCD_WriteText("OFF");
		
	else if ( t_x == (91 * 40) )
		LCD_WriteText("ON");
		
	else
		putdec16( t_x / 40 );
		
	LCD_WriteData('<');
	
	if ( t_x && t_x != (91 * 40) )
		LCD_WriteText(" (sec)");


	if ( sel_state ){
		sel_state = 0;
		
		if ( sel == SEL_T_ON ){
			
			if( t_on && ( t_on <= (90 * 40) ) ){	//Zakres pracy pompy z interwalami
				sel = SEL_T_OFF;	//ustawiamy czas t_off
				LCD_Clear();
			}
			else{	//Pompa wlaczona albo wylaczona na stale
			
				eeprom_write_word ( &eep_pump_time_on, g_pump_time_on_set = t_on);	//Przepisujemy t_on
				menu = MENU_PARAMS;
				menu_init = 1;
				repeat_enable = 0;	//Wylaczamy autorepetycje klawiszy
				
			}	
		
		}
		else{
			
			eeprom_write_word ( &eep_pump_time_on, g_pump_time_on_set = t_on);		//Przepisujemy t_on
			eeprom_write_word ( &eep_pump_time_off, g_pump_time_off_set = t_off);	//Przepisujemy t_off
			menu = MENU_PARAMS;
			menu_init = 1;
			repeat_enable = 0;	//Wylaczamy autorepetycje klawiszy
		
		}

		sel++;

	}
	if ( up_state ){
		up_state = 0;
	
		if ( sel == SEL_T_ON ){		//Próg gorny ( 91 * 40 )
		
			if ( t_on < ( 91 * 40 )  )
				t_on += 40;	//krok 25ms * 40 = 1 sec
			
		}
		else{						//Próg gorny ( 90 * 40 )
			
			if ( t_off < ( 90 * 40 )  )
			t_off += 40;	//krok 25ms * 40 = 1 sec
			
		}
		LCD_Clear();
	
	}
	if ( down_state ){	
		down_state = 0;
	
		if ( sel == SEL_T_ON ){		//Próg dolny( 0 * 40 )
		
			if ( t_on )
				t_on -= 40;	//krok 25ms * 40 = 1 sec
				
		}else{						//Próg dolny( 1 * 40 )
			
			if ( t_off > (1 * 40) )
				t_off -= 40;	//krok 25ms * 40 = 1 sec
			
		}
	
		LCD_Clear();
	
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Ustawianie histerezy
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_hysteresis( void ){
	
	static uint16_t hyst;

	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
	
		LCD_Clear();
		hyst = g_hysteresis_set;
		repeat_enable = 1;		//Wlaczamy autorepetycje klawiszy
	
	}

	//Proces menu :

	LCD_GoTo(3,0);
	LCD_WriteText("Histereza");

	LCD_GoTo(4,1);
	print_temperature( hyst, SEL_ALL );

	if ( sel_state ){
		sel_state = 0;
	
		eeprom_write_word ( &eep_hysteresis, g_hysteresis_set = hyst );
		menu = MENU_PARAMS;
		menu_init = 1;
		repeat_enable = 0;	//Wylaczamy autorepetycje klawiszy

	}
	
	if ( up_state ){	//Próg gorny 5.0 C
		up_state = 0;
	
		if ( hyst < 50 )
			hyst++;
	
		LCD_Clear();
	
	}
	
	if ( down_state ){	//Próg dolny 0.1 C
		down_state = 0;
	
		if ( hyst > 1 )
			hyst--;
	
		LCD_Clear();
	
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//					   Menu rozgrzewania roztworu
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		m_heat_start ( void ){
	
	static uint8_t init;
	
	if( menu_init ){
		menu_init = 0;
		//Inicjalizacja menu :
		
		countdown_enable = 0;	//Wlaczamy potrzebne procesy
		temp_read_enable = 1;
		
		heat_enable();
		g_pump_time_on = 0;	//Wylaczamy pompe
		g_set_temp = temp_set;
		g_hysteresis = g_hysteresis_set;
		init = 0;
		
	}
	
	//Proces menu :
	
	if( temp_update ){
		temp_update = 0;
		
		if ( temp_curr >= ( temp_set - g_hysteresis_set ) ){
			
			if( !init ){
				
				init = 1;
				
				LCD_Clear();			//Ekran menu
				LCD_GoTo(2,0);
				LCD_WriteText("Wcisnij SEL");
				LCD_GoTo(5,1);
				print_time( time_mm_set, time_ss_set, SEL_NONE, 0 );
				buzzer_on ( BUZ_FREQ_HZ( 4000 ), BUZ_TIME_MS( 200 ) );
				
			}
			
			if ( sel_state ){
				sel_state = 0;
				
				menu = MENU_ETCHING;
				menu_init = 1;
				
			}
			
		}else{
		
			LCD_Clear();			//Ekran menu
			LCD_GoTo(0,0);
			LCD_WriteText("Rozgrzewanie ...");
			LCD_GoTo(0,1);
			LCD_WriteText("Temp. ");
			print_temperature( temp_curr, SEL_NONE );
		
		}
		
	}
	
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//						Obsluga przycisku zasilania
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	on_power_pressed( void ){
	
	uint8_t i = 0;
	
	power_on ^= 1;	// Wlacz / Wylacz
	
	if ( !power_on ){
		
		while( ! ( IN_POWER & (1 << PIN_POWER) ) );	//Czekamy na puszczenie przycisku zasilania
	
	//Wylaczamy przerwania
		cli();
	//Tekst wylaczenia
		LCD_Clear();
		LCD_GoTo( 2,1 );
		LCD_WriteText( "KM-Circuits" );
		
		LCD_Home();
		LCD_WriteText( "Wylaczanie" );
		
		while( (i++) < 3 ){
			_delay_ms( 250 );
			LCD_WriteText( " ." );
		}
		_delay_ms( 250 );
	//Wylaczamy wszystko . . .
		LCD_Clear();
		LCD_off();
		soft_reset();	//RESET
		
	}else{
		//Wczytujemy zmienne z eeprom
		
		g_pump_time_off_set		= eeprom_read_word(&eep_pump_time_off);
		g_pump_time_on_set		= eeprom_read_word(&eep_pump_time_on);
		g_hysteresis_set		= eeprom_read_word(&eep_hysteresis);
		temp_set				= eeprom_read_word(&eep_set_temp);
		time_ss_set				= eeprom_read_word(&eep_time_ss);
		time_mm_set				= eeprom_read_word(&eep_time_mm);
		
	}
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//						Zakonczenie odliczania
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		on_end_countdown ( void ){
	
	menu = MENU_MAIN;
	menu_init = 1;
	
	buzzer_on ( BUZ_FREQ_HZ( 4000 ), BUZ_TIME_MS( 2500 ) );
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//					Odliczanie czasu trawienia
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void		countdown_process ( void ){
		
	if ( countdown_enable ){
		
		time_ss--;
		if ( time_ss == -1 ){
			
			time_ss = 59;
			time_mm--;
			
		}
			
		if ( !time_mm && !time_ss ){
			
			on_end_countdown();
			countdown_enable = 0;
		
		}
		
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Wyswietla czas
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	print_time ( uint8_t mm, uint8_t ss, uint8_t sel, uint8_t zero_print ){
	
	if ( sel == SEL_MIN )
		LCD_WriteData('>');
		
	if ( (mm < 10) && zero_print )
		LCD_WriteData('0');
		
	putdec16( mm );
	
	if ( sel == SEL_MIN )
	LCD_WriteData('<');
	
	LCD_WriteData(':');
	
	if ( sel == SEL_SEC )
	LCD_WriteData('>');
	
	if ( ss<10 )
		LCD_WriteData('0');
		
	putdec16( ss );
	
	if ( sel == SEL_SEC )
	LCD_WriteData('<');
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//					   Wyswietla liczbe 16 bitowa
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

uint8_t	putdec16 ( uint16_t b ){

	uint8_t tab[ 8 ];
	uint8_t l = 0;
	uint8_t i;
	
	do{
		
		tab[l] = ( ( b % 10 ) + '0' );
		l++;
		
	}while ( b /= 10 );
	
	i = l;
	
	while( l-- )
	LCD_WriteData( tab[l] );
	
	return	i;
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Wyswietla temperature
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	print_temperature ( int16_t temp, uint8_t sel ){
	
	if ( sel == SEL_ONES || sel == SEL_ALL )
		LCD_WriteData('>');
	
	putdec16( temp / 10 );
	
	if ( sel == SEL_ONES )
		LCD_WriteData('<');
	
	LCD_WriteData('.');
	
	if ( sel == SEL_TENS )
		LCD_WriteData('>');
	
	putdec16( temp % 10 );
	
	if ( sel == SEL_TENS || sel == SEL_ALL )
		LCD_WriteData('<');
	
	LCD_WriteData( 0xDF );
	LCD_WriteData( 'C' );
		
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Zapelnia obszar pamieci zerami spod adresu ( dest ) 
//		o wielkosci ( size )
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	memzero ( uint8_t *dest, uint8_t size ){

	while ( size-- ){
		
		*dest++ = 0;
		
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Kopiuje obszar pamieci ( src ), o wielkosci ( size ) do ( dest )
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	memcpy	( uint8_t *src, uint8_t *dest, uint8_t size ){

	while ( size-- )
	*(dest++) = *(src++);

}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Przygotowywuje TIMER 0 do pracy prescaler /8
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	timer0_init( void ){
	
	TCCR0 |= (1<<CS01);	// prescaler /8
	TIMSK |= (1<<TOIE0);	
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Przygotowywuje TIMER 2 do pracy prescaler /256
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void	timer2_init( void ){
	
	TCCR2 |= ( (1<<CS22) | (1<<CS21) );	// prescaler /256
	TIMSK |= (1<<TOIE2);
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Procedura obslugi przerwania TIMER'a 0
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

ISR( TIMER0_OVF_vect ){
	
	//Przerwanie wywolywane co 256us
	
	buttons_process();
	if( power_on ){
		buzzer_process();
	}
	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//		Procedura obslugi przerwania TIMER'a 2 ( czasomierza )
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

ISR( TIMER2_OVF_vect ){
	
	//Przerwanie wywolywane co 8ms
	TCNT2 = 6;	//okres TCNT - 250 cykli
	
	static uint8_t cnt = 125;
	
	if ( !( --cnt ) ){
		cnt = 125;
	
	//	Okres 1 sec
	
		countdown_process();

		time_update = 1;
	
	}
	
}
