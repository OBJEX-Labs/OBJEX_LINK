#include <Arduino.h>
#include <Wire.h>
#include <RadioLib.h>
#include <Adafruit_NeoPixel.h>
#include <PD_UFP.h>

#define LED_PIN    48
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define FUSB302_INT_PIN   10
PD_UFP_Log_c PD_UFP;
int voltage = 5;

int key = 20;
int S1 = 27;
int S2 = 28;

//#define MASTER_NODE //ANCHOR module 1 with INITIATING_NODE and module 2 with "//INITIATING_NODE"

int LORA_SS = 5;	 // LORA SPI CS
int LORA_SCLK = 45;	 // LORA SPI CLK
int LORA_MISO = 35;	 // LORA SPI MISO
int LORA_MOSI = 36;	 // LORA SPI MOSI

// SX1262 has the following connections:
SX1262 radio = new Module(5, 47, 4, 46);

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

//flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

void setFlag(void) {
  // we sent or received a packet, set the flag
  operationDone = true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // SDA | SCL
  PD_UFP.init_PPS(FUSB302_INT_PIN, PPS_V(5.0), PPS_A(3.0));
  pinMode(34, INPUT_PULLUP);

  pinMode(key, INPUT);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);

  led.begin();           
  led.show();            
  led.setBrightness(50);

  SPI.begin(LORA_SCLK, LORA_MISO, LORA_MOSI, LORA_SS);

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  //int state = radio.begin(); 
  radio.setDio2AsRfSwitch(true);
  if (radio.setDio2AsRfSwitch() != RADIOLIB_ERR_NONE) {
    Serial.println(F("Failed to set DIO2 as RF switch!"));
    while (true);
  }
  int state = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0, true);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  #if defined(MASTER_NODE)
    // send the first packet on this node
    Serial.print(F("[SX1262] Sending first packet ... "));
    transmissionState = radio.startTransmit("OBJEX Link 1 Ready!");
    transmitFlag = true;
    led.setPixelColor(0, led.Color(255,0,0)); //red
    led.show();
  #else
    // start listening for LoRa packets on this node
    Serial.print(F("[SX1262] Starting to listen ... "));
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
    } else {
      Serial.print(F("failed, code "));
      Serial.println(state);
      while (true);
    }
    led.setPixelColor(0, led.Color(0,255,0)); //green
    led.show();
  #endif
}

int myFlag = 0;
String temp;

void loop() {
  PD_UFP.run();
  PD_UFP.print_status(Serial); 

  #if defined(MASTER_NODE)
    if(!digitalRead(34)){
      while (!digitalRead(34)){
        delay(500);
      }
      Serial.print(F("[SX1262] Sending another packet ... "));
      if(!myFlag){
        transmissionState = radio.startTransmit("1");
        myFlag = 1;
      }else{
        transmissionState = radio.startTransmit("0");
        myFlag = 0;
      }
      
    }
  #else
     String str;
      int state = radio.readData(str);

      if (state == RADIOLIB_ERR_NONE) {
        if(str == "1"){
          PD_UFP.set_PPS(PPS_V(10.0), PPS_A(3.0)); 
          led.setPixelColor(0, led.Color(0,0,255));
          led.show();
        }else if(str == "0"){
          PD_UFP.set_PPS(PPS_V(5.0), PPS_A(3.0)); 
          led.setPixelColor(0, led.Color(255,0,0));
          led.show();
        }
      }
  #endif
}