#include "lib.h"

// activate power of channel
bool POWER[CHANNEL_COUNT];

bool read(SWITCH sw) {
  return digitalRead(sw) == LOW ? true : false;
}

void power_apply() {
    uchar relay_dat;

    for (int i = 0; i < CHANNEL_COUNT; i++) {
      if (POWER[i]) 
        bitSet(relay_dat, i);
      else
        bitClear(relay_dat, i);
    }
    
    //ground latchPin and hold low for as long as you are transmitting
    digitalWrite(latchPin, LOW);  

    shiftOut(dataPin, clockPin, MSBFIRST, relay_dat); 

    //return the latch pin high to signal chip that it 
    //no longer needs to listen for information
    digitalWrite(latchPin, HIGH);

}
