///////////////////////////////////////////////////////
// LIBRARY FOR YDSCO SD600 BASED ADDRESSABLE LED STRIP
// (c) 2012 Jason Hotchkiss 
// http://hotchk155.blogspot.co.uk/

#ifndef _S600_H_
#define _S600_H_



// Macro to assemble an RGB colour
#define RGB(r,g,b) (\
(((unsigned long)(g))<<16)|\
(((unsigned long)(b))<<8)|\
(((unsigned long)(r)))\
)


class sd600
{
public:
	sd600(int numLeds);
	static void begin();
	static void begin_transfer();
	static int is_transfer_complete();
	static void refresh();
	static void cls();
	static void set(int index, unsigned long colour);
	static unsigned long get(int index);
};


#endif
