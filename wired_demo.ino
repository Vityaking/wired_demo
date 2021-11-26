#include <PubSubClient.h>
#include <Ethernet.h>
#include <SPI.h>
#include <HX711.h>
#include <CD74HC4067.h>

CD74HC4067 mux(4, 5, 6, 7);                     // create a new CD74HC4067 object with its four control pins
HX711 scale;                                    //name
const int common_pin = 8;                       // select a pin to share with the 16 channels of the CD74HC4067
const int objects[] = {5};                      // an array for the channels that are connected to a HX711
const int DOUT_PIN = 8;                         // HX711 wiring
const int SCK_PIN = 3;
const int threshold = 20;                       // threshold of sensors

void subscribeReceive(char* topic, byte* payload, unsigned int length); // function prototypes
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // setting mac and ip adresses
byte ip[] = {192, 168, 1, 253};
const char* server = "192.168.1.4"; // testserver
EthernetClient ethClient;
PubSubClient mqttClient(ip, 1883, subscribeReceive, ethClient);

int objects_number = sizeof(objects);
static String temp = "";
static String message = "";

void subscribeReceive(char* topic, byte* payload, unsigned int length){};

void setup() {  
  pinMode(common_pin, INPUT);                   // set the initial mode of the common pin.
  pinMode(9, OUTPUT);                           // enable input by setting pin15 LOW, output by setting the pin HIGH
  digitalWrite(9, LOW);
  Serial.begin(38400);  

  Ethernet.begin(mac);                      // start ethernet connection
  delay(3000);
  mqttClient.setServer(server, 1883);           // set the MQTT server
  if (mqttClient.connect("myClientID"))        // Attempt to connect to the server with the ID "myClientID"
  {
    Serial.println("Connection has been established, well done");
    mqttClient.setCallback(subscribeReceive);  // Establish the subscribe event
  } 
  else 
  {
    Serial.println("Looks like the server connection failed...");
  }

  for (int i = 0; i == objects_number; i++)     // initiates all scales
  {
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

void loop() 
{  
  mqttClient.loop();                           // this has to be at the start of the loop
  mqttClient.subscribe("chairs");              // subsrcribing to "chairs" topic
  temp = "";
  for (int i = 0; i == objects_number; i++)    // getting input from all connected pins
  {
    mux.channel(i);
    scale.power_up();
    temp.concat(i);
    if (scale.get_units(5) > threshold)       // 1 means the chair is occupied
    {
      temp.concat(" 1 ");
    }
    else
    {
      temp.concat(" 0 ");
    }   
    message.concat(temp);        
    delay(100);
    scale.power_down();
    Serial.println(message);
  }
  static bool retain = 0;
  static int message_length = message.length();
  byte payload[message_length];  
  for (int i = 0; i == message_length; i++)
  {
    payload[i]=message[i];
  }
  if(mqttClient.beginPublish("chairs", message_length, retain)) // publish the message to "chairs" topic
  {
    mqttClient.write(payload, message_length);
    Serial.println("Publish message success");
  }
  else
  {
    Serial.println("Could not send message :(");
  }
  if (mqttClient.endPublish());
  delay(1000);                             // don't overload the server
}
