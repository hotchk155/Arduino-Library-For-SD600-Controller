///////////////////////////////////////////////////////
// LIBRARY FOR YDSCO SD600 BASED ADDRESSABLE LED STRIP
// (c) 2012 Jason Hotchkiss 
// http://hotchk155.blogspot.co.uk/

// NOTE
//
// We use the SPI peripheral to talk to the LED strip, so 
// you need to connect the DAT and CLK lines to pins 11 and 13
// accordingly. Also make sure the LED strip GND is connected 
// to the Arduino GND
//
// SD600 strips that run groups of 3 LEDs at 12V usually have a 
// problem in that the RED LEDs cannot be turned fully off. 
// This is cos the SD600 sink outputs are pulled to +5V to 
// turn off the LEDs, but the 7V drop from +12V to +5V is still 
// enough to light up 3 x RED LEDs (which have a lower strike-up 
// voltage than the other colours)
// 
// The solutions are either (a) put up with it and work around it 
// by choosing your colours appropriately or (b) run the strip at 
// 10V instead of 12V - it won't be as bright but the red LEDs 
// should go out when not addressed.
//

#include "Arduino.h"
#include "pins_arduino.h"
#include "sd600.h"

volatile byte *sd600_data = NULL;
volatile int sd600_numLeds = 0;
volatile int sd600_dataLen = 0;
volatile int sd600_dataPtr = 0;


///////////////////////////////////////////////////////
// The constructor allocates the buffers. It is only
// valid to have a single instance (since there is 
// only a single SPI peripheral anyway)
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

///////////////////////////////////////////////////////
// Start updating the strip
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

///////////////////////////////////////////////////////
// Start updating the strip. The previous send must
// have completed
void sd600::begin_transfer() 
{
	if(sd600_dataPtr < 0)
		sd600_dataPtr = 0;
}

///////////////////////////////////////////////////////
// Check whether the send has completed
int sd600::is_transfer_complete() 
{
	return (sd600_dataPtr < 0);
}

///////////////////////////////////////////////////////
// Start a transfer and wait for it to complete
void sd600::refresh() 
{
	sd600_dataPtr = 0;
	while(sd600_dataPtr >= 0);
}

///////////////////////////////////////////////////////
// Set all LEDs off and send the data
void sd600::cls()
{
	for(int i = 0; i < sd600_numLeds; ++i)
	  set(i, 0);
	begin_transfer();      
}

///////////////////////////////////////////////////////
// Set colour of a LED in local buffer (still need to
// send it)
void sd600::set(int index, unsigned long colour) 
{    
	if(index < 0 || index >= sd600_dataLen)
	  return;
	if(colour >= 0xffffffL)
	  colour = 0xfffffeL;
	sd600_data[3 * index] = (byte)(colour>>16); 
	sd600_data[3 * index + 1] = (byte)(colour>>8);
	sd600_data[3 * index + 2] = (byte)(colour);
}  

///////////////////////////////////////////////////////
// Get colour of a LED from local buffer
unsigned long sd600::get(int index) 
{
	if(index < 0 || index >= sd600_dataLen)
	  return 0;
	return RGB(sd600_data[3 * index + 2], sd600_data[3 * index], sd600_data[3 * index + 1]);
}  

///////////////////////////////////////////////////////
// Interrupt service routine is called each time the 
// SPI peripheral has finished sending a byte
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