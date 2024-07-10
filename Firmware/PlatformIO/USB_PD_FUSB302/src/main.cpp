#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <PD_UFP.h>

#define LED_PIN    48
#define LED_COUNT 1

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define FUSB302_INT_PIN   10
#define userbtn 34

PD_UFP_Log_c PD_UFP;

float voltage = 5.0;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // SDA | SCL
  //PD_UFP.init(FUSB302_INT_PIN, PD_POWER_OPTION_MAX_POWER);
  PD_UFP.init_PPS(FUSB302_INT_PIN, PPS_V(5.0), PPS_A(5.0));

  pinMode(userbtn, INPUT_PULLUP);

  led.begin();           
  led.show();            
  led.setBrightness(50); 
}

void loop() {
  PD_UFP.run();
  PD_UFP.print_status(Serial); 

  if(!digitalRead(userbtn)){
    while (!digitalRead(userbtn)){
      delay(500);
    }
    if(voltage <= 21.0){ 
      voltage += 0.5;
      PD_UFP.set_PPS(PPS_V(voltage), PPS_A(5.0));
    }else if(voltage >= 21.0){ 
      voltage = 5.0;
      PD_UFP.set_PPS(PPS_V(5.0), PPS_A(5.0));  
    }
  }

  if(voltage>=5.0 && voltage <= 8){
    led.setPixelColor(0, led.Color(0,255,0));
    led.show();
  }if(voltage>=10.0 && voltage <= 15.5){
    led.setPixelColor(0, led.Color(255,255,0));
    led.show();
  }if(voltage>=16.0){
    led.setPixelColor(0, led.Color(255,0,0));
    led.show();
  }
}