//-------------------------------------------------------------------------------------------------
// Wyœwietlacz alfanumeryczny ze sterownikiem HD44780
// Sterowanie w trybie 4-bitowym bez odczytu flagi zajêtoœci
// z dowolnym przypisaniem sygna³ów steruj¹cych
//-------------------------------------------------------------------------------------------------

#include <avr/io.h>
#include <util/delay.h>

//******** Definicje pinow *****************************************************************************

#define	PIN_RS		PORTC0
#define	PIN_E		PORTC1
#define	PIN_DB4		PORTC2
#define	PIN_DB5		PORTC3
#define	PIN_DB6		PORTC4
#define	PIN_DB7		PORTC5
#define	PIN_LCDON	PORTB7

//		Rejestr wyjsciowy ( PORT ) :

#define	OUT_RS		PORTC
#define	OUT_E		PORTC
#define	OUT_DB4		PORTC
#define	OUT_DB5		PORTC
#define	OUT_DB6		PORTC
#define	OUT_DB7		PORTC
#define	OUT_LCDON	PORTB

//		Rejestr kierunku ( DDR ) :

#define	DDR_RS		*( &OUT_RS - 1 )
#define	DDR_E		*( &OUT_E - 1 )
#define	DDR_DB4		*( &OUT_DB4 - 1 )
#define	DDR_DB5		*( &OUT_DB5 - 1 )
#define	DDR_DB6		*( &OUT_DB6 - 1 )
#define	DDR_DB7		*( &OUT_DB7 - 1 )
#define	DDR_LCDON	*( &OUT_LCDON - 1 )

//		Rejestr wejsciowy ( PIN ) :

#define	IN_RS		*( &OUT_RS - 2 )
#define	IN_E		*( &OUT_E - 2 )
#define	IN_DB4		*( &OUT_DB4 - 2 )
#define	IN_DB5		*( &OUT_DB5 - 2 )
#define	IN_DB6		*( &OUT_DB6 - 2 )
#define	IN_DB7		*( &OUT_DB7 - 2 )
#define	IN_LCDON	*( &OUT_LCDON - 2 )

//-------------------------------------------------------------------------------------------------
//
// Instrukcje kontrolera Hitachi HD44780
//
//-------------------------------------------------------------------------------------------------

#define HD44780_CLEAR					0x01

#define HD44780_HOME					0x02

#define HD44780_ENTRY_MODE				0x04
	#define HD44780_EM_SHIFT_CURSOR		0
	#define HD44780_EM_SHIFT_DISPLAY	1
	#define HD44780_EM_DECREMENT		0
	#define HD44780_EM_INCREMENT		2

#define HD44780_DISPLAY_ONOFF			0x08
	#define HD44780_DISPLAY_OFF			0
	#define HD44780_DISPLAY_ON			4
	#define HD44780_CURSOR_OFF			0
	#define HD44780_CURSOR_ON			2
	#define HD44780_CURSOR_NOBLINK		0
	#define HD44780_CURSOR_BLINK		1

#define HD44780_DISPLAY_CURSOR_SHIFT	0x10
	#define HD44780_SHIFT_CURSOR		0
	#define HD44780_SHIFT_DISPLAY		8
	#define HD44780_SHIFT_LEFT			0
	#define HD44780_SHIFT_RIGHT			4

#define HD44780_FUNCTION_SET			0x20
	#define HD44780_FONT5x7				0
	#define HD44780_FONT5x10			4
	#define HD44780_ONE_LINE			0
	#define HD44780_TWO_LINE			8
	#define HD44780_4_BIT				0
	#define HD44780_8_BIT				16

#define HD44780_CGRAM_SET				0x40

#define HD44780_DDRAM_SET				0x80

//-------------------------------------------------------------------------------------------------
//
// Deklaracje funkcji
//
//-------------------------------------------------------------------------------------------------

void	LCD_WriteCommand	(unsigned char);
void	LCD_WriteData		(unsigned char);
void	LCD_WriteText		(char *);
void	LCD_GoTo			(unsigned char, unsigned char);
void	LCD_Clear			(void);
void	LCD_Home			(void);
void	LCD_init			(void);

inline	void	LCD_on		(void){ OUT_LCDON |= (1<<PIN_LCDON); }	//Zarzadzanie podswietleniem LCD
inline	void	LCD_off		(void){ OUT_LCDON &= ~(1<<PIN_LCDON); }

//-------------------------------------------------------------------------------------------------
//
// Koniec pliku HD44780.h
//
//-------------------------------------------------------------------------------------------------
