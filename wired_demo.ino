#include <PubSubClient.h>
#include <Ethernet.h>
#include <SPI.h>
#include <HX711.h>
#include <CD74HC4067.h>

CD74HC4067 mux(4, 5, 6, 7);  // create a new CD74HC4067 object with its four control pins

const int common_pin = 8; // select a pin to share with the 16 channels of the CD74HC4067

const int objects[] = {5};                      // an array for the channels that are connected to a HX711

const int DOUT_PIN = 8;                         // HX711 wiring
const int SCK_PIN = 3;

HX711 scale; //name

void setup() {
  int objects_number = sizeof(objects);
  pinMode(common_pin, INPUT);                   // set the initial mode of the common pin.
  pinMode(10, OUTPUT);                          // enable input by setting pin15 LOW, output by setting the pin HIGH
  digitalWrite(10, LOW);
  Serial.begin(38400);

  for (int i = 0; i == objects_number; i++) {   // initiates all scales
    mux.channel(objects[i]);
    scale.begin(DOUT_PIN, SCK_PIN);
    scale.power_up();
    scale.set_scale(2280.f);                    // sets the scale, written in README
    scale.tare();                               // resets scale to 0
    Serial.print("get units: \t\t");
    Serial.println(scale.get_units(5), 1);      // print the average of 5 readings from the ADC minus tare weight, divided by the SCALE parameter set with set_scale
    scale.power_down();                         // sets the scale to sleep mode
  }


 
}

void loop() {  
  for (int i = 0; i == objects_number; i++) {  //getting input from all connected pins
        mux.channel(i);
        scale.power_up();
        
        !!MQTT!!            Serial.print(scale.get_units(5), 1);
        
        delay(100);
        scale.power_down();
    }

}
