#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define DEBUG

#include <Arduino.h>

#define uchar unsigned char 
#define uint  unsigned int

// how fast the strobe mode blinks
#define STROBE_SPEED 75 //orig 75

// blinker on/off speed
#define BLINK_SPEED 400

// how long to do alert mode after horn is finished
#define HORN_ALERT_TIMEOUT 3000

// how quickly to switch off alert mode when alert switch is off
#define DEFAULT_ALERT_TIMEOUT 500

// Inputs (numbers taken from example code)
const int INPUT1 = 2;
const int INPUT2 = 3;
const int INPUT3 = 4;
const int INPUT4 = 5;
const int INPUT5 = 6;
const int INPUT6 = 7;
const int INPUT7 = 9;
const int INPUT8 = 11;

const int K1 = 12;
const int K2 = 10;
const int K3 = 8;
const int K4 = A0;


// INPUTS
#define SW_1 INPUT1
#define SW_2 INPUT2
#define SW_3 INPUT3
#define SW_BLINK_L INPUT4
#define SW_RED INPUT5
#define SW_HORN INPUT6
#define SW_BLINK_R INPUT7

#define SW_ALERT SW_RED
// #define SW_DRL SW_3
#define SW_POS SW_3
#define SW_BOX_LIGHT SW_2
#define SW_4WAY SW_1

// number of relays on the board
#define CHANNEL_COUNT 8

//OUTPUTS (0-based)
#define CHANNEL_POS_L 1
#define CHANNEL_POS_R 2
#define CHANNEL_BLINK_L 3
#define CHANNEL_BLINK_R 4
#define CHANNEL_LIGHT_L 5
#define CHANNEL_LIGHT_R 6
#define CHANNEL_BOX_LIGHT 7
#define CHANNEL_HORN 8


#endif /* __SETTINGS_H__ */