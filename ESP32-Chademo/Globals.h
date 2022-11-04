#ifndef GLOBALS_H_
#define GLOBALS_H_

#define CHADEMO_IN1 4
#define CHADEMO_IN2 7

#define CHADEMO_OUT1 8//??
#define CHADEMO_OUT2 9//??
#define CHADEMO_OUT3 10//??

#define MCP2515_CS 17// CS input of MCP2515
#define MCP2515_INT 23 // INT output of MCP2515
#define MCP2515_SCK 22
#define MCP2515_QUARTZ_FREQUENCY 16000000

#define CAN_BAUD 500000
#define min(a, b)           (((a) < (b)) ?  (a) : (b))
#define EEPROM_VALID	0xEE

//These have been moved to eeprom. After initial compile the values will be read from EEPROM.
//These thus set the default value to write to eeprom upon first start up
#define MAX_CHARGE_V	158
#define MAX_CHARGE_A	130
#define TARGET_CHARGE_V	160
#define MIN_CHARGE_A	20
#define CAPACITY 180

typedef struct
{
  uint8_t valid; //a token to store EEPROM version and validity. If it matches expected value then EEPROM is not reset to defaults //0
  float ampHours; //floats are 4 bytes //1
  float kiloWattHours; //5
  float packSizeKWH; //9
  uint16_t maxChargeVoltage; //21
  uint16_t targetChargeVoltage; //23
  uint8_t maxChargeAmperage; //25
  uint8_t minChargeAmperage; //26
  uint8_t capacity; //27
  uint8_t debuggingLevel; //29
} EESettings;

extern EESettings settings;
extern float Voltage;
extern float Current;
extern unsigned long CurrentMillis;
extern int Count;

#endif
