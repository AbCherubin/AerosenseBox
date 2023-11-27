#ifndef _Ab_WIEGAND_H
#define _Ab_WIEGAND_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class Ab_WIEGAND
{

public:
	Ab_WIEGAND();
	void begin();
	void begin(int pinD0, int pinD1);
	void begin(int pinD0, int pinIntD0, int pinD1, int pinIntD1);
	bool available();
	unsigned long getCode();
	int getWiegandType();

private:
	static void ReadD0();
	static void ReadD1();
	static bool DoWiegandConversion();
	static unsigned long GetCardId(volatile unsigned long *codehigh, volatile unsigned long *codelow, char bitlength);

	static volatile unsigned long _cardTempHigh;
	static volatile unsigned long _cardTemp;
	static volatile unsigned long _lastWiegand;
	static volatile int _bitCount;
	static int _wiegandType;
	static unsigned long _code;
};

#endif
