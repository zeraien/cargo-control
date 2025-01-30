#ifndef __LIB_H__
#define __LIB_H__

#include "settings.h"

typedef unsigned char SWITCH;
typedef unsigned char CHANNEL;

typedef enum {
  MODE_OFF,
  MODE_ON,
  MODE_BLINK,
  MODE_STROBE,
  MODE_UNSET
} e_mode;

typedef struct {
  float start_in;
  float duration;
  e_mode mode;
  bool phase;
} s_mode_timer;

//TODO not used yet
typedef struct {
  float start_in;
  float duration;
  e_mode next_mode;
  e_mode mode;
  bool phase;
} s_output_status;
//END not used yet

//Pin connected to latch of Digital Tube Module
const int latchPin = A3;
//Pin connected to clock of Digital Tube Module
const int clockPin = A4;
//Pin connected to data of Digital Tube Module
const int dataPin = A5;
//Pin connected to 595_OE of Digital Tube Module
const int OE_595 = A2;

// activate power of channel
extern bool POWER[CHANNEL_COUNT];

typedef struct {
  bool blink_left;
  bool blink_right;
  bool horn;
  bool alert;
  bool box_light;
  bool position_light;
  bool four_way;
  bool drl;
} SWITCH_STATUS;




bool read(SWITCH sw);
void power_apply();

#endif /* __LIB_H__ */