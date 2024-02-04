/*
ds18b20 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ds18b20.h"

//******** Procedury ***********************************************************************************

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//						Resetuje linie czujnika
//		
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

uint8_t ds18b20_reset() {
	uint8_t i;

	//low for 480us
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(480);

	//release line and wait for 60uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(60);

	//get value and wait 420us
	i = (DS18B20_PIN & (1<<DS18B20_DQ));
	_delay_us(420);

	//return the read value, 0=ok, 1=error
	return i;
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Zapis bitu
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void ds18b20_writebit(uint8_t bit){
	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//if we want to write 1, release the line (if not will keep low)
	if(bit)
		DS18B20_DDR &= ~(1<<DS18B20_DQ); //input

	//wait 60uS and release the line
	_delay_us(60);
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Odczyt bitu
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

uint8_t ds18b20_readbit(void){
	uint8_t bit=0;

	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//release line and wait for 14uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(14);

	//read the value
	if(DS18B20_PIN & (1<<DS18B20_DQ))
		bit=1;

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Zapis bajtu
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

void ds18b20_writebyte(uint8_t byte){
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1);
		byte >>= 1;
	}
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//							Odczyt bajtu
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

uint8_t ds18b20_readbyte(void){
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit()<<7);
	}
	return n;
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//			Obliczanie CRC8, wielomian [ X^8 + X^5 + X^4 + 1 ] (0x8C)
//
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

uint8_t		ds18b20_crc ( const uint8_t *addr, uint8_t len ){
	
	uint8_t crc = 0;
	
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;	
}

// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
//
//						Pobranie biezacej temperatury 
//			( 0x6000 ) - brak czujnika,  ( 0x7000 ) - Blad CRC
// # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

int16_t	ds18b20_gettemp() {
	uint8_t		temperature_l;
	uint8_t		temperature_h;
	uint8_t		scratchpad [ 9 ];
	int16_t		retd = 0;

	if ( ds18b20_reset() ) //reset
		return	DS18B20_NO_SENSOR;
		
	ds18b20_writebyte( DS18B20_CMD_SKIPROM ); //skip ROM
	ds18b20_writebyte( DS18B20_CMD_CONVERTTEMP ); //start temperature conversion

	while( !ds18b20_readbit() ); //wait until conversion is complete
	
	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD); //read scratchpad

	//read 9 byte from scratchpad
	scratchpad[0] = temperature_l = ds18b20_readbyte();//TEMP L
	scratchpad[1] = temperature_h = ds18b20_readbyte();//TEMP H
	scratchpad[2] = ds18b20_readbyte();
	scratchpad[3] = ds18b20_readbyte();
	scratchpad[4] = ds18b20_readbyte();
	scratchpad[5] = ds18b20_readbyte();
	scratchpad[6] = ds18b20_readbyte();
	scratchpad[7] = ds18b20_readbyte();
	scratchpad[8] = ds18b20_readbyte();	//CRC
	
	if ( scratchpad[8] != ds18b20_crc( scratchpad, 8 ) )
		return	DS18B20_CRC_ERR;
	
	int16_t tmp = 0;
	
	retd = ( ( temperature_h << 4 ) & 0x70 ) + ( temperature_l >> 4 ); //Czesc calkowita

	tmp += ( temperature_l & 0x01 ) ? ( 625 )	: ( 0 );	//Konwersja czesci ulamkowej
	tmp += ( temperature_l & 0x02 ) ? ( 1250 )	: ( 0 );
	tmp += ( temperature_l & 0x04 ) ? ( 2500 )	: ( 0 );
	tmp += ( temperature_l & 0x08 ) ? ( 5000 )	: ( 0 );
	
	tmp += 5000;	//Zaokraglenie do czesci dziesietnych
	
	if ( tmp > 10000 ){	//zwiekszanie czesci calkowitej w przypadku przepelnienia
		retd++;
		tmp -= 10000;
	}
		
	retd *= 10;				//Czesc calkowita ma wage	10
	retd += ( tmp / 1000 );	//a czesci dziesietne		1
	
	if ( temperature_h & 0x80 )	//temp ujemne
		retd = 0 - retd;
	
	return (retd + 15 );	//Temperatura pomnozona przez 10 i zaokraglona do cz. dziesietnych ( i 1.5stopnia korekty )
}

