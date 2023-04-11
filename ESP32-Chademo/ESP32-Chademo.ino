#include "Globals.h"
#include "Chademo.h"
#include "ISAShunt.h"
#include "ChademoWebServer.h"
#include "WebSocketPrint.h"

#include <SPIFFS.h>
#include <ACAN_ESP32.h>
#include <ACAN2515.h>
#include <SPI.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define LED_PIN 2
#define HOSTNAME "ESP32-Chademo"

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
int socketMessage = 0;
uint8_t soc;
extern bool overrideStart1 = false;

ISA Sensor;
ACAN2515 can1 (MCP2515_CS, SPI, MCP2515_INT) ;
EESettings settings;
ChademoWebServer chademoWebServer(settings);
WebSocketPrint wsPrint(chademoWebServer.getWebSocket());
CHADEMO chademo(wsPrint);

String cmdStr;
byte Command = 0; // "z" will reset the AmpHours and KiloWattHours counters

uint16_t uint16Val;
uint16_t uint16Val2;
uint8_t uint8Val;
uint8_t print8Val;
uint8_t help8Val;

volatile uint8_t timerIntCounter = 0;
volatile uint8_t timerFastCounter  = 0;
volatile uint8_t timerChademoCounter = 0;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  
  timerFastCounter++;
  timerChademoCounter++;
  if (timerChademoCounter >= 3)
  {
    timerChademoCounter = 0;
    if (chademo.bChademoMode  && chademo.bChademoSendRequests) chademo.bChademoRequest = 1;
  }

  if (timerFastCounter == 8)
  {
    timerFastCounter = 0;
    timerIntCounter++;
    if (timerIntCounter == 18)
    {
      timerIntCounter = 0;
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void setup() {
  Serial.begin(115200);
  delay(4000);

  Serial.println("ESP32-CHADEMO");
  
  pinMode(CHADEMO_IN2, INPUT);
  pinMode(CHADEMO_IN1, INPUT);
  pinMode(CHADEMO_OUT1, OUTPUT);
  pinMode(CHADEMO_OUT2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  //onboard can
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  ACAN_ESP32_Settings canSettings(CAN_BAUD);
  canSettings.mRxPin = GPIO_NUM_16;
  canSettings.mTxPin = GPIO_NUM_17;
  uint16_t errorCode = ACAN_ESP32::can.begin(canSettings);
  if (errorCode > 0) {
    Serial.print ("Can0 Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
  SPI.begin(MCP2515_SCK, MCP2515_MISO, MCP2515_MOSI, MCP2515_CS) ;
  ACAN2515Settings settings2515 (MCP2515_QUARTZ_FREQUENCY, CAN_BAUD);
  errorCode = can1.begin(settings2515, [] { can1.isr () ; });
  if (errorCode > 0) {
    Serial.print ("Can1 Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
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
    settings.currentMissmatch = true;
    Save();
  }
  //set to false on every boot.
  overrideStart1 = false;
  
  help8Val = 1;
  print8Val = 1;

  updateTargetAV();

  WiFi.mode(WIFI_AP);
  WiFi.hostname(HOSTNAME);
  WiFi.begin();

  chademoWebServer.setup();

  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  timer = timerBegin(0, 80, true);
  /* Attach onTimer function to our timer */
  timerAttachInterrupt(timer, &onTimer, true);
 
  /* Set alarm to call onTimer function every second 1 tick is 1us
  => 1 second is 1000000us */
  /* Repeat the alarm (third parameter) */
  timerAlarmWrite(timer, 25000, true);
  timerAlarmEnable(timer);
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
  bool sent = can1.tryToSend(outFrame);
  if (settings.debuggingLevel > 1)
  {
    Serial.print(F("CAR: VCU Wake Up: "));
    Serial.println(sent);

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

void RngErr() {
  Serial.println(F("Range Error"));
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
  Serial.println(F("CAPAH - Sets pack amp-hours, shunt provides AmpHours consumed"));
  Serial.println(F("CAPKWH - Sets pack kilowatt-hours"));
  Serial.println(F("DBG - Sets debugging level"));
  Serial.println(F("BMS - Sets use to 0 - No BMS, 1 - ESP32 BMS for SoC and cell voltage and temeratures"));
  Serial.println(F("MISS - Sets use to 0 - Disable Current Miss-Match check, 1 - Enable Current Miss-Match check"));


  Serial.println(F("Example: V=395 - sets CHAdeMO voltage target to 395"));
}

void ParseCommand() {
  float fltVal = 0;
  if (cmdStr == "V") {
    uint16Val = Serial.parseInt();
    if (uint16Val > 0 && uint16Val < 1000) {
      settings.targetChargeVoltage = uint16Val;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "VMAX") {
    uint16Val = Serial.parseInt();
    if (uint16Val > 0 && uint16Val < 1000) {
      settings.maxChargeVoltage = uint16Val;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "AMIN") {
    uint8Val = Serial.parseInt();
    if (uint8Val > 0 && uint8Val < 256) {
      settings.minChargeAmperage = uint8Val;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "AMAX") {
    uint8Val = Serial.parseInt();
    if (uint8Val > 0 && uint8Val < 256) {
      settings.maxChargeAmperage = uint8Val;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "CAPAH") {
    uint8Val = Serial.parseInt();
    if (uint8Val > 0 && uint8Val < 256) {
      settings.capacity = uint8Val;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "BMS") {
    uint8Val = Serial.parseInt();
    if (uint8Val >= 0 && uint8Val < 2) {
      settings.useBms = uint8Val == 1;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "MISS") {
    uint8Val = Serial.parseInt();
    if (uint8Val >= 0 && uint8Val < 2) {
      settings.currentMissmatch = uint8Val == 1;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "CAPKWH") {
    fltVal = Serial.parseFloat();
    if (fltVal > 0 && fltVal < 100) {
      settings.packSizeKWH = fltVal;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "DBG") {
    uint8Val = Serial.parseInt();
    if (uint8Val >= 0 && uint8Val < 4) {
      settings.debuggingLevel = uint8Val;
      Save();
      print8Val = 1;
    } else {
      RngErr();
    }
  }
  else if (cmdStr == "SAVE") {
    Save();
  }
  else if (cmdStr == "FULLRESET") {
    Serial.println(F("Resetting EEPROM"));
    settings.valid = 0;
    Save();
  }
  else {
    help8Val = 1;
  }
}


void SerialCommand()
{
  int available = Serial.available();
  if (available > 0)
  {
    Command = Serial.peek();
    
    if (Command == 'p')
    {
      print8Val = 1;
    }
    else if (Command == 'u')
    {
      updateTargetAV();
    }
    else if (Command == 'h') {
      help8Val = 1;
    }
    else {
      cmdStr = Serial.readStringUntil('=');
      ParseCommand();
    }
    while (Serial.available() > 0) Serial.read();
  }
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
  Serial.print (F("BMS "));
  Serial.println (settings.useBms, 1);
  Serial.print (F("MISS "));
  Serial.println (settings.currentMissmatch, 1);
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
  Serial.print (F("IN1"));
  Serial.print (!digitalRead(CHADEMO_IN1) > 0 ? F(":1 ") : F(":0 "));
  Serial.print (F("IN2"));
  Serial.print (!digitalRead(CHADEMO_IN2) > 0 ? F(":1 ") : F(":0 "));
  Serial.print (overrideStart1 > 0 ? F(":1 ") : F(":0 "));
  Serial.print (F("OVER1"));
  Serial.print (F("CHG T: "));
  Serial.println (CurrentMillis / 1000 - ChargeTimeRefSecs);
}

void broadcastMessage() {
  DynamicJsonDocument json(1024);
  char buffer[1024]; // create temp buffer
  switch(socketMessage) {
    case 0: {
      json["voltage"] = Sensor.Voltage;
      json["amperage"] = Sensor.Amperes;
      json["power"] = Sensor.KW;
      json["ampHours"] = Sensor.AH;
      json["soc"] = soc;

      size_t len = serializeJson(json, buffer);  // serialize to buffe
      chademoWebServer.getWebSocket().textAll(buffer, len);
    }
    case 1: {
      //chademo status
      json["chademoState"] = chademo.getState();
      json["EVSEAvailableVoltage"] = chademo.getEVSEParams().availVoltage;
      json["EVSEAvailableCurrent"] = chademo.getEVSEParams().availCurrent;
      json["EVSEThresholdVoltage"] = chademo.getEVSEParams().thresholdVoltage;

      size_t len = serializeJson(json, buffer);  // serialize to buffe
      chademoWebServer.getWebSocket().textAll(buffer, len);
    }
    case 2: {
      //car status
      json["OVER1"] = overrideStart1;
      json["OUT1"] = digitalRead(CHADEMO_OUT1);
      json["OUT2"] = digitalRead(CHADEMO_OUT2);
      json["IN1"] = !digitalRead(CHADEMO_IN1);
      json["IN2"] =!digitalRead(CHADEMO_IN2);

      size_t len = serializeJson(json, buffer);  // serialize to buffe
      chademoWebServer.getWebSocket().textAll(buffer, len);
    }
  }
  socketMessage++;
  if (socketMessage > 2) {
    socketMessage = 0;
  }
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
  chademoWebServer.execute();
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
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      Count = 0;
      SerialCommand();
      sendStatusToVCU();
      broadcastMessage();

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
  }
  if (can1.receive(inFrame)) {
    Sensor.handleCANFrame(inFrame);
    if (settings.useBms) {
        if (inFrame.id == 0x355) {
           soc = (inFrame.data[1] << 8) + inFrame.data[0];
           chademo.setStateOfCharge(soc);
        }
    }

  }
}
