/*
  Power delivery PPS 3.0 DEMO
  
  This code is designed to manage a USB-C Power Delivery (PD) system and control the RGB status LED. 
  It initializes serial communication, I2C communication, and the FUSB320 controller for handling USB-C PD. 
  The code monitors a user button to increase the output voltage by 0.5V with each press, cycling back to 5V after reaching 21V.
  It also uses the builtin RGB LED to display different colors based on the requested voltage: 
    green for 5V-8V, 
    yellow for 10V-15.5V, 
    red for 16V and above
*/

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <PD_UFP.h>
 
#define LED_PIN    48   // GPIO connected to RGB status led (WS2812B)
#define LED_COUNT   1

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);  // Create a Adafruit_NeoPixel object to control the LED
 
#define FUSB302_INT_PIN 10  //Interrupt pin is used to signal important events related to the operation of the USB-C port like cable insertion or removal and data packet reception
#define USER_BTN 34         //Onboard user button BTN
PD_UFP_Log_c PD_UFP;
 
float voltage = 5.0;
 
void setup() {
  // Start serial communication
  Serial.begin(115200);
  Serial.println("Power delivery PPS 3.0 DEMO");

  //Initialize I2C communication
  Wire.begin(8, 9); // SDA | SCL

  // Initialize FUSB320 controller library
  PD_UFP.init_PPS(FUSB302_INT_PIN, PPS_V(5.0), PPS_A(5.0));

  pinMode(USER_BTN, INPUT_PULLUP);  // Set user button as input with pull-up resistor

  // Initialize NeoPixel library
  led.begin();  // Initialize NeoPixel led object
  led.show();   // Turn OFF LED
  led.setBrightness(100); 
}
 
void loop() {
  //NOTE: The PD_UFP.run() function must be called in a short interval,
  //less than 10ms, to ensure state machine response to negotiation message in time. 
  //Long response time may result in a power reset cycle initiated by USB PD hosts.
  PD_UFP.run();
  PD_UFP.print_status(Serial); 

  // At each press of the user button BTN, the voltage is increased by 0.5V
  // Once the maximum voltage of 21V is reached, the required voltage is set to 5V
  if(!digitalRead(USER_BTN)){

    while (!digitalRead(USER_BTN)){
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

  //The color of the RGB LED present in the ELPM module is modified 
  //according to the voltage supplied
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