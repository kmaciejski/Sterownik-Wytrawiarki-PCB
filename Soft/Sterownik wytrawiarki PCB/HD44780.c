//-------------------------------------------------------------------------------------------------
// Wyœwietlacz alfanumeryczny ze sterownikiem HD44780
// Sterowanie w trybie 4-bitowym bez odczytu flagi zajêtoœci
// z dowolnym przypisaniem sygna³ów steruj¹cych
//-------------------------------------------------------------------------------------------------

#include "HD44780.h"
//-------------------------------------------------------------------------------------------------
//
// Funkcja wystawiaj¹ca pó³bajt na magistralê danych
//
//-------------------------------------------------------------------------------------------------
void	_LCD_OutNibble(unsigned char nibbleToWrite)
{
	if(nibbleToWrite & 0x01)
		OUT_DB4 |= (1<<PIN_DB4);
	else
		OUT_DB4 &= ~(1<<PIN_DB4);

	if(nibbleToWrite & 0x02)
		OUT_DB5 |= (1<<PIN_DB5);
	else
		OUT_DB5 &= ~(1<<PIN_DB5);

	if(nibbleToWrite & 0x04)
		OUT_DB6 |= (1<<PIN_DB6);
	else
		OUT_DB6 &= ~(1<<PIN_DB6);

	if(nibbleToWrite & 0x08)
		OUT_DB7 |= (1<<PIN_DB7);
	else
		OUT_DB7 &= ~(1<<PIN_DB7);
}
//-------------------------------------------------------------------------------------------------
//
// Funkcja zapisu bajtu do wyœwietacza (bez rozró¿nienia instrukcja/dane).
//
//-------------------------------------------------------------------------------------------------
void	 _LCD_Write(unsigned char dataToWrite)
{
	OUT_E |= (1<<PIN_E); //  E = 1
	_LCD_OutNibble(dataToWrite >> 4);
	OUT_E &= ~(1<<PIN_E); // E = 0
	OUT_E |= (1<<PIN_E); //  E = 1
	_LCD_OutNibble(dataToWrite);
	OUT_E &= ~(1<<PIN_E); // E = 0
	_delay_us(50);
}
//-------------------------------------------------------------------------------------------------
//
// Funkcja zapisu rozkazu do wyœwietlacza
//
//-------------------------------------------------------------------------------------------------
void	 LCD_WriteCommand(unsigned char commandToWrite)
{
	OUT_RS &= ~(1<<PIN_RS); // RS = 0
	_LCD_Write(commandToWrite);
}
//-------------------------------------------------------------------------------------------------
//
// Funkcja zapisu danych do pamiêci wyœwietlacza
//
//-------------------------------------------------------------------------------------------------
void	LCD_WriteData(unsigned char dataToWrite)
{
	OUT_RS |= (1<<PIN_RS); // RS = 1
	_LCD_Write(dataToWrite);
}
//-------------------------------------------------------------------------------------------------
//
// Funkcja wyœwietlenia napisu na wyswietlaczu.
//
//-------------------------------------------------------------------------------------------------
void	 LCD_WriteText(char * text)
{
	while(*text)
	LCD_WriteData(*text++);
}

//-------------------------------------------------------------------------------------------------
//
// Funkcja ustawienia wspó³rzêdnych ekranowych
//
//-------------------------------------------------------------------------------------------------
void	LCD_GoTo(unsigned char x, unsigned char y)
{
	LCD_WriteCommand(HD44780_DDRAM_SET | (x + (0x40 * y)));
}
//-------------------------------------------------------------------------------------------------
//
// Funkcja czyszczenia ekranu wyœwietlacza.
//
//-------------------------------------------------------------------------------------------------
void	 LCD_Clear(void)
{
	LCD_WriteCommand(HD44780_CLEAR);
	_delay_ms(2);
}
//-------------------------------------------------------------------------------------------------
//
// Funkcja przywrócenia pocz¹tkowych wspó³rzêdnych wyœwietlacza.
//
//-------------------------------------------------------------------------------------------------
void	LCD_Home(void)
{
	LCD_WriteCommand(HD44780_HOME);
	_delay_ms(2);
}
//-------------------------------------------------------------------------------------------------
//
// Procedura inicjalizacji kontrolera HD44780.
//
//-------------------------------------------------------------------------------------------------
void	 LCD_init(void)
{
	unsigned char i;
	DDR_LCDON |= (1<<PIN_LCDON);
	DDR_DB4 |= (1<<PIN_DB4); // Konfiguracja kierunku pracy wyprowadzeñ
	DDR_DB5 |= (1<<PIN_DB5); //
	DDR_DB6 |= (1<<PIN_DB6); //
	DDR_DB7 |= (1<<PIN_DB7); //
	DDR_E 	|= (1<<PIN_E);   //
	DDR_RS 	|= (1<<PIN_RS);  //
	_delay_ms(55); // oczekiwanie na ustalibizowanie siê napiecia zasilajacego
	OUT_RS &= ~(1<<PIN_RS); // wyzerowanie linii RS
	OUT_E &= ~(1<<PIN_E);  // wyzerowanie linii E

	for(i = 0; i < 3; i++) // trzykrotne powtórzenie bloku instrukcji
	  {
		  OUT_E |= (1<<PIN_E); //  E = 1
		  _LCD_OutNibble(0x03); // tryb 8-bitowy
		  OUT_E &= ~(1<<PIN_E); // E = 0
		  _delay_ms(5); // czekaj 5ms
	  }

	OUT_E |= (1<<PIN_E); //  E = 1
	_LCD_OutNibble(0x02); // tryb 4-bitowy
	OUT_E &= ~(1<<PIN_E); // E = 0

	_delay_ms(1); // czekaj 1ms 
	LCD_WriteCommand(HD44780_FUNCTION_SET | HD44780_FONT5x7 | HD44780_TWO_LINE | HD44780_4_BIT); // interfejs 4-bity, 2-linie, znak 5x7
	LCD_WriteCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_OFF); // wy³¹czenie wyswietlacza
	LCD_WriteCommand(HD44780_CLEAR); // czyszczenie zawartosæi pamieci DDRAM
	_delay_ms(2);
	LCD_WriteCommand(HD44780_ENTRY_MODE | HD44780_EM_SHIFT_CURSOR | HD44780_EM_INCREMENT);// inkrementaja adresu i przesuwanie kursora
	LCD_WriteCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK); // w³¹cz LCD, bez kursora i mrugania

	LCD_WriteText("Wytrawiarka PCB");
	LCD_GoTo( 2,1 );
	LCD_WriteText( "KM-Circuits" );
	LCD_on();
	
	_delay_ms(1000);

}

//-------------------------------------------------------------------------------------------------
//
// Koniec pliku HD44780.c
//
//-------------------------------------------------------------------------------------------------
