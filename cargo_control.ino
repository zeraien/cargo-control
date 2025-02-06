// Cargo Bike Light Controller
// Copyright 2024 Dimo Fedortchenko
// MIT License
//
// Controls lights on your cargo bike with the DB22D08 board
// from http://www.eletechsup.com 


#include <FlexiTimer2.h>
#include "settings.h"
#include "lib.h"
#include "read_inputs.h"

long previousMillis = 0;  // for calculating delta time

e_mode MODE[CHANNEL_COUNT];
e_mode PREV_MODE[CHANNEL_COUNT];
s_mode_timer MODE_TIMERS[CHANNEL_COUNT];
SWITCH_STATUS switch_status = {false, false, false, false, false, false, false, true};

// strobe helpers
uint STROVE_CURRENT_TICK[CHANNEL_COUNT];

float ALERT_TIMER = 0;
float BLINK_TIMER = 0;
float STROBE_TIMER = 0;
bool ACTIVE_BLINK_PHASE = 0;

// blink and strobe helpers
/*
Determines the phase of the blinkers and strobes.
There are currently only two phases, 0 and 1.
In order for two lights to blink at different times,
we can set the phase to 0 for one light and 1 for another,
this will mean that when the light with phase 0 is on, the light
with phase 1 is off and vice versa. This applies to strobing effects
slightly differently, mainly in that the strobing effect works with a
few rapid blinks followed by an equally long pause, using phase we can
alternate the timing of the strobe phase vs the pause phase.
*/
uchar BLINK_PHASE[CHANNEL_COUNT];

/* This determines the appearance of the strobe effect.
A conter keeps track of which tick we are currently on.
A tick is a single step in the strobe pattern.
A tick can both mean the light is on or off, depending on 
if the tick counter is odd or even.
For active ticks of 6, this means the light will switch on 3 times
and switch off 3 times.
The total ticks is how long the total strobe cycle is.
Generally the total cycle is double the number of active ticks.
During the active phase the lights will switch on and off rapidly,
and during the inactive phase, the lights will be off.

Using BLINK_PHASE we can then determine if the active phase for a specific
light is during the first half or second half of the total tick counter.
*/
uchar strobe_active_ticks = 6;
uchar strobe_total_ticks = strobe_active_ticks * 2;

// convert 1-based channel number to 0-based array index
uchar chtoi(uchar ch) {
  return ch - 1;
}
uchar itoch(uchar i) {
  return i + 1;
}

void set_mode(CHANNEL ch, e_mode mode) {
  if (ch > 0 && ch <= CHANNEL_COUNT)
  {
    MODE[ch - 1] = mode;
  }
}

void set_power(CHANNEL ch, bool on) {
  set_mode(ch, on ? MODE_ON : MODE_OFF);
}

// default phase: 0
void set_strobe(CHANNEL ch, bool on, int phase = 0) {
  set_mode(ch, on ? MODE_STROBE : MODE_OFF);
  BLINK_PHASE[chtoi(ch)] = phase;
}

void set_blink(CHANNEL ch, bool on, int phase = 0) {
  set_mode(ch, on ? MODE_BLINK : MODE_OFF);
  BLINK_PHASE[chtoi(ch)] = phase;
}

void apply_switches() {

  bool is_blink = switch_status.blink_left || switch_status.blink_right;

  // horn ///////////////////////////////////////////////
  set_power(CHANNEL_HORN, switch_status.horn);
  set_power(CHANNEL_BOX_LIGHT, switch_status.box_light);

  if (switch_status.horn) {
    ALERT_TIMER = HORN_ALERT_TIMEOUT;
  }
  ///////////////////////////////////////////////////////

  // blinkers /////////////////////////////////////////
  // right blinker
  set_blink(CHANNEL_BLINK_R, switch_status.blink_right || (!is_blink && switch_status.four_way));
  // left blinker
  set_blink(CHANNEL_BLINK_L, switch_status.blink_left || (!is_blink && switch_status.four_way));
  /////////////////////////////////////////////////////


  // DRL //////////////////////////////////////////////
  if (switch_status.four_way)
  {
    set_blink(CHANNEL_LIGHT_L, switch_status.drl && !is_blink, 1);
    set_blink(CHANNEL_LIGHT_R, switch_status.drl && !is_blink, 1);
  }
  else
  {
    set_power(CHANNEL_LIGHT_L, switch_status.drl && !switch_status.blink_left);
    set_power(CHANNEL_LIGHT_R, switch_status.drl && !switch_status.blink_right);
  }
  /////////////////////////////////////////////////////


  // Position lights //////////////////////////////////
  if (switch_status.blink_left)
    set_blink(CHANNEL_POS_L, true);
  else
    set_power(CHANNEL_POS_L, switch_status.position_light && switch_status.drl);

  if (switch_status.blink_right)
    set_blink(CHANNEL_POS_R, true);
  else
    set_power(CHANNEL_POS_R, switch_status.position_light && switch_status.drl);
  /////////////////////////////////////////////////////


  // Strobe Mode //////////////////////////////////////
  if (switch_status.alert) {
    ALERT_TIMER = DEFAULT_ALERT_TIMEOUT;
  }

  if (ALERT_TIMER > 0) {
    set_strobe(CHANNEL_POS_L, true, 1);
    set_strobe(CHANNEL_POS_R, true, 0);
    if (!is_blink) {
      set_strobe(CHANNEL_LIGHT_L, true, 1);
      set_strobe(CHANNEL_LIGHT_R, true);
      set_strobe(CHANNEL_BLINK_L, true);
      set_strobe(CHANNEL_BLINK_R, true, 1);
    }
  }
  ///////////////////////////////////////////////////////

}

void subtract_timers(unsigned long dt) {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (MODE_TIMERS[i].start_in > 0)
    {
      MODE_TIMERS[i].start_in -= dt;
    } else if (MODE_TIMERS[i].duration > 0) {
      MODE_TIMERS[i].duration -= dt;
    }
  }
  
  if (ALERT_TIMER > 0)
    ALERT_TIMER -= dt;
  
  if (BLINK_TIMER > 0) {
    BLINK_TIMER -= dt;
  }
  else {
    BLINK_TIMER = BLINK_SPEED;
    ACTIVE_BLINK_PHASE = !ACTIVE_BLINK_PHASE;
  }

  if (STROBE_TIMER > 0){
    STROBE_TIMER -= dt;
  }
  else {
    STROBE_TIMER = STROBE_SPEED;
    for (int i = 0; i < CHANNEL_COUNT; i++) {
      if (STROVE_CURRENT_TICK[i] >= strobe_total_ticks)
        STROVE_CURRENT_TICK[i] = 1;
      else 
        STROVE_CURRENT_TICK[i]++;
    }
  }
}

void mode_apply() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {

    if (MODE_TIMERS[i].mode != MODE_UNSET && MODE_TIMERS[i].start_in <= 0)
    {
      if (MODE_TIMERS[i].duration <= 0) {
        MODE_TIMERS[i].mode = MODE_UNSET;
        MODE[i] = MODE_OFF;
      } else {
        MODE[i] = MODE_TIMERS[i].mode;
      }
    }

    switch (MODE[i]) {
      case MODE_ON:
        POWER[i] = true;
        break;
      case MODE_OFF:
        POWER[i] = false;
        break;
      case MODE_STROBE:
        strobe_mode_for_index(i);
        break;
      case MODE_BLINK:
        blink_mode_for_index(i);
        break;
    }
    PREV_MODE[i] = MODE[i];
  }
}

void blink_mode_for_index(uchar index) {
  bool reset = PREV_MODE[index] != MODE_BLINK;

  POWER[index] = ACTIVE_BLINK_PHASE == BLINK_PHASE[index];
}

void strobe_mode_for_index(uchar index) {
  bool phase = (bool)BLINK_PHASE[index];
  
  // only lit when strobe count is even
  bool odd = (STROVE_CURRENT_TICK[index] % 2) > 0;

  bool first_half = STROVE_CURRENT_TICK[index] <= strobe_active_ticks && STROVE_CURRENT_TICK[index] > 0;

  POWER[index] = !odd && ((phase && first_half) || (!phase && !first_half));
}


void setup() {

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    POWER[i] = false;
    STROVE_CURRENT_TICK[i] = 0;
    MODE[i] = MODE_OFF;
    PREV_MODE[i] = MODE_OFF;
    BLINK_PHASE[i] = 0;
    MODE_TIMERS[i] = {0, 0, MODE_UNSET, 0};
  }

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(OE_595, OUTPUT);

  pinMode(K1, INPUT);
  pinMode(K2, INPUT);
  pinMode(K3, INPUT);
  pinMode(K4, INPUT);

  pinMode(INPUT1, INPUT);
  pinMode(INPUT2, INPUT);
  pinMode(INPUT3, INPUT);
  pinMode(INPUT4, INPUT);
  pinMode(INPUT5, INPUT);
  pinMode(INPUT6, INPUT);
  pinMode(INPUT7, INPUT);
  pinMode(INPUT8, INPUT);


  digitalWrite(INPUT1, HIGH);
  digitalWrite(INPUT2, HIGH);
  digitalWrite(INPUT3, HIGH);
  digitalWrite(INPUT4, HIGH);
  digitalWrite(INPUT5, HIGH);
  digitalWrite(INPUT6, HIGH);
  digitalWrite(INPUT7, HIGH);
  digitalWrite(INPUT8, HIGH);

  digitalWrite(K1, HIGH);
  digitalWrite(K2, HIGH);
  digitalWrite(K3, HIGH);
  digitalWrite(K4, HIGH);

  digitalWrite(OE_595, LOW);// Enable 74HC595  


  FlexiTimer2::set(50, 1.0/1000, custom_loop); // call every 50ms
  FlexiTimer2::start();
 
  start_up();  

}

// todo
// void set_next_mode(CHANNEL channel, e_mode mode, long start_in, long duration)
// {

// }

void start_up() {
  float start = 0.;
  float duration = 500.;

  MODE_TIMERS[chtoi(CHANNEL_LIGHT_L)] = {start, duration, MODE_STROBE, 0};
  MODE_TIMERS[chtoi(CHANNEL_LIGHT_R)] = {start, duration, MODE_STROBE, 0};

  start+=500;

  MODE_TIMERS[chtoi(CHANNEL_POS_L)] = {start, duration, MODE_STROBE, 0};
  MODE_TIMERS[chtoi(CHANNEL_POS_R)] = {start, duration, MODE_STROBE, 0};

  start+=1000;

  MODE_TIMERS[chtoi(CHANNEL_BLINK_L)] = {start, duration, MODE_STROBE, 0};
  MODE_TIMERS[chtoi(CHANNEL_BLINK_R)] = {start, duration, MODE_STROBE, 0};
}

void custom_loop(void) {
  apply_switches();
  mode_apply();
  power_apply();

  // display is currently not used
  //TODO find a way to switch off the display
  //tube_display_loop();
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long dt = currentMillis - previousMillis;
  previousMillis = currentMillis;

  switch_status = read_inputs();
  subtract_timers(dt);
}