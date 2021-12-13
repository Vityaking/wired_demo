#include <PubSubClient.h>
#include <Ethernet.h>
#include <SPI.h>
#include <HX711.h>
#include <CD74HC4067.h>
#include <IPAddress.h>

CD74HC4067 mux(2, 5, 6, 7);                     // create a new CD74HC4067 object with its four control pins
HX711 scale;                                    //name
const int common_pin = 8;                       // select a pin to share with the 16 channels of the CD74HC4067
const int objects[] = {5};                      // an array for the channels that are connected to a HX711
const int objects_number = sizeof(objects)/sizeof(int);
static int states[objects_number];				// in this array the previous read of the given sensor is stored, false value filtering
const int DOUT_PIN = 8;                         // HX711 wiring
const int SCK_PIN = 3;
const int threshold = 20;                       // threshold of sensors

void subscribeReceive(char* topic, byte* payload, unsigned int length); // function prototypes
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // setting mac and ip adresses

IPAddress ip(192, 168, 0, 102);
IPAddress Server(192, 168, 0, 100); 
EthernetClient ethClient;
PubSubClient mqttClient(Server, 1883, subscribeReceive, ethClient);

void subscribeReceive(char* topic, byte* payload, unsigned int length){};

void setup() 
{  
	pinMode(4, OUTPUT);                           //setting the SD card pin to HIGH hoping it would solve the problem
	digitalWrite(4, HIGH);
	pinMode(common_pin, INPUT);                   // set the initial mode of the common pin.
	pinMode(9, OUTPUT);                           // enable input by setting mux pin15 LOW, output by setting the pin HIGH
	digitalWrite(9, LOW);
	digitalWrite(10, HIGH);
	Serial.begin(38400);  
	while (!Serial)                               // wait for serial port to open
	{
		;
	}
	Ethernet.init(10);  						  // pin10 is the SS pin	
	mqttClient.setClient(ethClient);
	mqttClient.setServer(Server, 1883);           // set the MQTT server
	if (Ethernet.begin(mac))                      // start ethernet connection
	{
		Serial.println("Successful connection");
	} 
	else
	{
		Serial.println("Unsuccessful connection");
	}                 
	delay(1500);

	ethClient.connect(Server, 1883);
	delay(1000);
	if (mqttClient.connect("arduino"))      	  // Attempt to connect to the server with the ID "arduino"
	{
		Serial.println("Connection has been established, well done");  
		mqttClient.setCallback(subscribeReceive);     // Establish the subscribe event  
	} 
	else 
	{
		Serial.println("Looks like the server connection failed...");
	}

	for (int i = 0; i < objects_number; i++)     // initiates all scales
	{
		mux.channel(objects[i]);
		scale.begin(DOUT_PIN, SCK_PIN);
		scale.power_up();
		scale.set_scale(2280.f);                    // sets the scale, written in README
		scale.tare();                               // resets scale to 0
		Serial.println(scale.get_units(5), 1);      // print the average of 5 readings from the ADC minus tare weight, divided by the SCALE parameter set with set_scale
		scale.power_down();                         // sets the scale to sleep mode
		Serial.println("scale is initialized");
	} 
}


void loop() 
{  
	mqttClient.loop();                           // this has to be at the start of the loop
	delay(1000);								 // temporary delay, will be deleted later
	mqttClient.subscribe("chairs");              // subsrcribing to "chairs" topic
	bool retain = 0;
	for (int i = 0; i < objects_number; i++)    // getting input from all connected pins
	{
		mux.channel(objects[i]);
		scale.power_up();
		if ((0<=objects[i]) && (objects[i]<10))
		{
			byte message[2];
			message[0] = '0' + objects[i];		// the message byte array will be sent to the MQTT server, its last digit shows the occupancy
			message[1] = '0';					// everything before the last digit will be the ID number
			float merleg = scale.get_units(5);
		if (merleg > threshold)       // 1 means the chair is occupied
		{
			if (states[i] == 1)
			{
				message[1] = '0' + 1;
				if (mqttClient.publish("chairs", message, (2*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]);
				states[i] = 1;
			}
			else 
			{
				message[1] = '0' + 0;
				if (mqttClient.publish("chairs", message, (2*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]);
				states[i] = 1;
			}
		}
		else
		{
			if (states[i] == 0)
			{
				message[1] = '0' + 0;
				if (mqttClient.publish("chairs", message, (2*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]);
				states[i] = 0;
			}
			else 
			{
				message[1] = '0' + 1;
				if (mqttClient.publish("chairs", message, (2*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]);
				states[i] = 0;
			}
		}
		}

		if ((10<=objects[i]) && (objects[i]<16))
		{
			byte message[3];
			message[0] = '0' + (objects[i]/10);
			message[1] = '0' + (objects[i]%10);
			message[2] = '0';
			float merleg = scale.get_units(5);
		if (merleg > threshold)       // 1 means the chair is occupied
		{
			if (states[i] == 1)
			{
				message[2] = '0' + 1;
				if (mqttClient.publish("chairs", message, (3*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]); Serial.print(message[2]);
				states[i] = 1;
			}
			else
			{
				message[2] = '0' + 0;
				if (mqttClient.publish("chairs", message, (3*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]); Serial.print(message[2]);
				states[i] = 1;
			}
		}
		else
		{
			if (states[i] == 0)
			{
				message[2] = '0' + 0;
				if (mqttClient.publish("chairs", message, (3*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]); Serial.print(message[2]);
				states[i] = 0;
			}
			else
			{
				message[2] = '0' + 1;
				if (mqttClient.publish("chairs", message, (3*sizeof(byte)), retain)) Serial.println("Publish message success");
				Serial.print(message[0]);Serial.print(message[1]); Serial.print(message[2]);
				states[i] = 0;
			}
		}
 		}
		delay(100);
		scale.power_down();
		Serial.print("states: ");
		Serial.println(states[0]);
	}
	if (mqttClient.endPublish());
	delay(1000);                             // don't overload the server
}
