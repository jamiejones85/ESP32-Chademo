#include "Globals.h"
#include "Chademo.h"
#include "ISAShunt.h"

#include <ACAN_ESP32.h>
#include <ACAN2515.h>
#include <SPI.h>
#include <EEPROM.h>
#include <ESP32TimerInterrupt.h>

const unsigned long Interval = 10;
unsigned long Time = 0;
unsigned long ChargeTimeRefSecs = 0; // reference time for charging.
unsigned long PreviousMillis = 0;
unsigned long CurrentMillis = 0;
float Voltage = 0;
float Current = 0;
float Power = 0;
float lastSavedAH = 0;
int Count = 0;

ISA Sensor;
ACAN2515 can1 (MCP2515_CS, SPI, MCP2515_INT) ;
EESettings settings;
ESP32Timer ITimer0(0);

uint16_t uint16Val;
uint16_t uint16Val2;
uint8_t uint8Val;
uint8_t print8Val;
uint8_t help8Val;

bool IRAM_ATTR TimerHandler0(void * timerNo)
{ 
timerFastCounter++;
  timerChademoCounter++;
  if (timerChademoCounter >= 3)
  {
    timerChademoCounter = 0;
    if (chademo.bChademoMode  && chademo.bChademoSendRequests) chademo.bChademoRequest = 1;
  }

  if (timerFastCounter == 8)
  {
    debugTick = 1;
    wifiTick = 1;
    timerFastCounter = 0;
    timerIntCounter++;
    if (timerIntCounter == 18)
    {
      timerIntCounter = 0;
    }
  }
}

void setup() {
  pinMode(CHADEMO_IN2, INPUT);
  pinMode(CHADEMO_IN1, INPUT);

  Serial.begin(115200);
  Serial.println("ESP32-CHADEMO");

  //onboard can
  ACAN_ESP32_Settings canSettings(CAN_BAUD);
  canSettings.mRxPin = GPIO_NUM_16;
  canSettings.mTxPin = GPIO_NUM_17;
  ACAN_ESP32::can.begin(canSettings);

  SPI.begin() ;
  ACAN2515Settings settings2515 (MCP2515_QUARTZ_FREQUENCY, CAN_BAUD);

  EEPROM.begin(sizeof(settings));
  EEPROM.get(0, settings);

  if (settings.valid != EEPROM_VALID) //not proper version so reset to defaults
  {
    settings.packSizeKWH = 33.0; //just a random guess. Maybe it should default to zero though?
    settings.valid = EEPROM_VALID;
    settings.ampHours = 0.0;
    settings.kiloWattHours = 0.0; // Is empty kiloWattHours count up
    settings.maxChargeAmperage = MAX_CHARGE_A;
    settings.maxChargeVoltage = MAX_CHARGE_V;
    settings.targetChargeVoltage = TARGET_CHARGE_V;
    settings.minChargeAmperage = MIN_CHARGE_A;
    settings.capacity = CAPACITY;
    settings.debuggingLevel = 1;
    Save();
  }
  help8Val = 1;
  print8Val = 1;

  if (ITimer0.attachInterruptInterval(25000, TimerHandler0))
  {
    Serial.print(F("Starting  ITimer0 OK, millis() = ")); Serial.println(millis());
  }

  updateTargetAV();

}

void updateTargetAV()
{
  chademo.setTargetAmperage(settings.maxChargeAmperage);
  chademo.setTargetVoltage(settings.targetChargeVoltage);
}


void Save()
{
  Serial.println("Saving EEPROM");
  noInterrupts();
  EEPROM.put(0, settings);
  EEPROM.commit();
  lastSavedAH = settings.ampHours;
  Serial.println (F("SAVED"));
  interrupts();
  delay(1000);
}

void sendStatusToVCU()
{
  CANMessage outFrame;
  outFrame.id = 0x354;
  outFrame.len = 8;

  outFrame.data[0] = 0x01; //Tell VCU Chademo Active
  outFrame.data[1] = chademo.getState();
  outFrame.data[2] = chademo.getEVSEStatus().presentCurrent;
  outFrame.data[3] = chademo.getEVSEStatus().presentVoltage;
  outFrame.data[4] = 0; //not used
  outFrame.data[5] = 0; //not used
  outFrame.data[6] = 0; //not used
  outFrame.data[7] = 0; //not used
  can1.tryToSend(outFrame);

  if (settings.debuggingLevel > 1)
  {
    Serial.print(F("CAR: VCU Wake Up"));
    timestamp();
  }

}

void timestamp()
{
  int milliseconds = (int) (millis() / 1) % 1000 ;
  int seconds = (int) (millis() / 1000) % 60 ;
  int minutes = (int) ((millis() / (1000 * 60)) % 60);
  int hours   = (int) ((millis() / (1000 * 60 * 60)) % 24);

  Serial.print(F(" Time:"));
  Serial.print(hours);
  Serial.print(F(":"));
  Serial.print(minutes);
  Serial.print(F(":"));
  Serial.print(seconds);
  Serial.print(F("."));
  Serial.println(milliseconds);
}

void printHelp() {
  help8Val = 0;
  Serial.println(F("Commands"));
  Serial.println(F("h - prints this message"));
  Serial.println(F("z - resets AH/KWH readings"));
  Serial.println(F("p - shows settings"));
  Serial.println(F("u - updates CHAdeMO settings"));
  Serial.println(F("AMIN - Sets min CHAdeMO current"));
  Serial.println(F("AMAX - Sets max CHAdeMO current"));
  Serial.println(F("V - Sets CHAdeMO voltage target"));
  Serial.println(F("VMAX - Sets CHAdeMO max voltage (for protocol reasons)"));
  Serial.println(F("CAPAH - Sets pack amp-hours"));
  Serial.println(F("CAPKWH - Sets pack kilowatt-hours"));
  Serial.println(F("DBG - Sets debugging level"));
  Serial.println(F("Example: V=395 - sets CHAdeMO voltage target to 395"));
}

void printSettings() {
  print8Val = 0;
  Serial.println (F("-"));
  Serial.print (F("AH "));
  Serial.println (settings.ampHours);
  Serial.print (F("KWH "));
  Serial.println (settings.kiloWattHours);
  Serial.print (F("AMIN "));
  Serial.println (settings.minChargeAmperage, 1);
  Serial.print (F("AMAX "));
  Serial.println (settings.maxChargeAmperage, 1);
  Serial.print (F("V "));
  Serial.println (settings.targetChargeVoltage, 1);
  Serial.print (F("VMAX "));
  Serial.println (settings.maxChargeVoltage, 1);
  Serial.print (F("CAPAH "));
  Serial.println (settings.capacity, 1);
  Serial.print (F("CAPKWH "));
  Serial.println (settings.packSizeKWH, 2);
  Serial.print (F("DBG "));
  Serial.println (settings.debuggingLevel, 1);
  Serial.println (F("-"));
}

void outputState() {

  Serial.print (F("ESP32: "));
  Serial.print (Voltage, 3);
  Serial.print (F("v "));
  Serial.print (Current, 2);
  Serial.print (F("A "));
  Serial.print (settings.ampHours, 1);
  Serial.print (F("Ah "));
  Serial.print (Power, 1);
  Serial.print (F("kW "));
  Serial.print (settings.kiloWattHours, 1);
  Serial.print (F("kWh "));
  Serial.print (F("OUT1"));
  Serial.print (digitalRead(CHADEMO_OUT1) > 0 ? F(":1 ") : F(":0 "));
  Serial.print (F("OUT2"));
  Serial.print (digitalRead(CHADEMO_OUT2) > 0 ? F(":1 ") : F(":0 "));
  Serial.print (F("OUT3"));
  Serial.print (digitalRead(CHADEMO_OUT3) > 0 ? F(":1 ") : F(":0 "));
  Serial.print (F("IN1"));
  Serial.print (digitalRead(CHADEMO_IN1) > 0 ? F(":1 ") : F(":0 "));
  Serial.print (F("IN2"));
  Serial.print (digitalRead(CHADEMO_IN2) > 0 ? F(":1 ") : F(":0 "));
  Serial.print (F("CHG T: "));
  Serial.println (CurrentMillis / 1000 - ChargeTimeRefSecs);
}

void loop() {

  uint8_t pos;
  CurrentMillis = millis();
  uint8_t len;
  CANMessage inFrame;
  float tempReading;

#ifdef DEBUG_TIMING
  if (debugTick == 1)
  {
    debugTick = 0;
    Serial.println(millis());
  }
#endif

  chademo.loop();

  if (CurrentMillis - PreviousMillis >= Interval)
  {
    Time = CurrentMillis - PreviousMillis;
    PreviousMillis = CurrentMillis;

    Count++;
    Voltage = Sensor.Voltage;
    Current = Sensor.Amperes;
    settings.ampHours = Sensor.AH;
    Power = Sensor.KW;

    //Count down kiloWattHours when drawing current.
    //Count up when charging (Current/Power is negative)
    settings.kiloWattHours = Sensor.KWH;

    chademo.doProcessing();

    if (Count >= 50)
    {

      Count = 0;
      #ifdef SIMPBMS
      chademo.setStateOfCharge(simpbms.getStateOfCharge());
      #endif
      sendStatusToVCU();

      if (print8Val > 0)
        printSettings();
      else
        outputState();
      if (help8Val > 0)
        printHelp();
      if (chademo.bChademoMode)
      {
        if (settings.debuggingLevel > 0) {
          Serial.print(F("Chademo Mode: "));
          Serial.println(chademo.getState());
        }
      }
    }
  }
  if (ACAN_ESP32::can.receive(inFrame)) {
    chademo.handleCANFrame(inFrame);
    Sensor.handleCANFrame(inFrame);
  }
}
