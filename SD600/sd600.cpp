///////////////////////////////////////////////////////
// LIBRARY FOR YDSCO SD600 BASED ADDRESSABLE LED STRIP
// (c) 2012 Jason Hotchkiss 
// blogspot.com/hotchk155

#include "Arduino.h"
#include "pins_arduino.h"
#include "sd600.h"

volatile byte *sd600_data = NULL;
volatile int sd600_numLeds = 0;
volatile int sd600_dataLen = 0;
volatile int sd600_dataPtr = 0;

sd600::sd600(int numLeds) 
{    

	// allocate the data buffer, if not done already
	if(!sd600_data) 
	{
	  sd600_numLeds = numLeds;
	  sd600_dataLen = 4 + numLeds * 3;
	  sd600_data = (byte*)malloc(sd600_dataLen);

	  // this is the set of 25 logic 1's at the end of 
	  // the data which cause all the data to be stored
	  sd600_data[sd600_dataLen - 4] = 0x7f;
	  sd600_data[sd600_dataLen - 3] = 0xff;
	  sd600_data[sd600_dataLen - 2] = 0xff;
	  sd600_data[sd600_dataLen - 1] = 0x80;
	  
	  // setting current data ptr to -1 means nothing
	  // is currently being transferred
	  sd600_dataPtr = -1;
	}
}

void sd600::begin() 
{
	// initialse the buffer and cue up a send
	cls();

	// set pin modes
	pinMode(SCK, OUTPUT);
	pinMode(MOSI, OUTPUT);
	pinMode(SS, OUTPUT); // must be output for SPI master mode

	digitalWrite(SCK, LOW);
	digitalWrite(MOSI, LOW);
	digitalWrite(SS, HIGH);

	// set SPI control register
	SPCR = 
		(1<<SPIE) | // 1 = Enable SPI interrupts
		(1<<SPE)  | // 1 = Enable SPI
		(0<<DORD) | // 0 = MSB first
		(1<<MSTR) | // 1 = SPI master mode
		(1<<CPOL) |
		(0<<CPHA) |
		(1<<SPR1) |
		(0<<SPR0);
	 SPSR = 0;

	// enable global interrupts       
	sei();
	 
	// this initial write to the SPI data register kicks off
	// the repeating transfer cycle
	SPDR=0;
}    

void sd600::begin_transfer() 
{
	sd600_dataPtr = 0;
}


int sd600::is_transfer_complete() 
{
	return (sd600_dataPtr < 0);
}

void sd600::refresh() 
{
	sd600_dataPtr = 0;
	while(sd600_dataPtr >= 0);
}

void sd600::cls()
{
	for(int i = 0; i < sd600_numLeds; ++i)
	  set(i, 0);
	begin_transfer();      
}

void sd600::set(int index, unsigned long colour) 
{    
	if(index < 0 || index > sd600_dataLen)
	  return;
	if(colour >= 0xffffffL)
	  colour = 0xfffffeL;
	sd600_data[3 * index] = (byte)(colour>>16); 
	sd600_data[3 * index + 1] = (byte)(colour>>8);
	sd600_data[3 * index + 2] = (byte)(colour);
}  

unsigned long sd600::get(int index) 
{
	if(index < 0 || index > sd600_dataLen)
	  return 0;
	return RGB(sd600_data[3 * index + 2], sd600_data[3 * index], sd600_data[3 * index + 1]);
}  

ISR (SPI_STC_vect)
{
	// are we idle?
	if(sd600_dataPtr < 0) 
	{
		// send a zero - the sd600 needs us to keep clocking the line
		// even when no data is being sent
		SPDR = 0;
	} 
	else 
	{
		// start sending the next data byte
		SPDR = sd600_data[sd600_dataPtr];

		// increment the data pointer, setting it back to -1 for 
		// idle mode when we reach the end of the buffer
		if(++sd600_dataPtr >= sd600_dataLen)
	  sd600_dataPtr = -1;
	}
}


