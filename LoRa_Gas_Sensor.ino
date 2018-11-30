/*
  Arduino Gas Sensor

  The data from a Grove gas sensor is transmitted with a RN2483A chip, via the LoRa network, to a TTN (The Things Network) account in which the Arduino device is registered.
  A dashboard is linked with the TTN account and allows the user to interact with the gas sensor to switch it on and off.
  A LED switches ON when the sensor is read, and OFF when it is not.
  Link for our dashboard : https://freeboard.io/board/Wauqxs
  Note 1 : Data are visible on this dashboard only if the associated NodeRed server is running
  Note 2 : We have also made a local dashboard with NodeRed dashboard

  The circuit:
    RN2483A chip
      TX          <-->     7
      RX          <-->     8
      3V3         <-->     3V3
      GND         <-->     GND

    Grove Gas Sensor (v1.4)
      SIG         <-->     A0
      VCC         <-->     5V
      GND         <-->     GND

    LED           <-->     2

  Created 30 november 2018
  By Sophie Rougeaux & Elie Taillardat
  In the context of the Innovative Smart Systems PTP - INSA Toulouse

  Project available on GitHub : https://github.com/elietaillardat/lora-mosh-project-5iss

*/

#include <SoftwareSerial.h>
#include <TheThingsNetwork.h>

// Set your AppEUI and AppKey
const char *devAddr = "26011253";
const char *nwkSKey = "D1A126A957FC1C25F20D3DB5379E3DE6";
const char *appSKey = "FC290B61D206CD05073F42CF673DE592";

// Pins
int sensorPin = A0;
#define LED_PIN           2
#define TX_PIN            7   //RN2483A TX pin
#define RX_PIN            8   //RN2483A RX pin

// Variables
bool ONreceived = true;
#define debugSerial Serial
SoftwareSerial loraSerial(TX_PIN, RX_PIN); //Serial link with the LoRa module
#define freqPlan TTN_FP_EU868              // Choose TTN_FP_EU868 (Europe) or TTN_FP_US915 (United States)

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);

/*
   SETUP
*/

void setup()
{
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  // Set callback for incoming messages
  ttn.onMessage(message);

  // Configure RN2483A using The Things Network library
  debugSerial.println("-- PERSONNALIZE");
  ttn.personalize(devAddr, nwkSKey, appSKey);

  debugSerial.println("-- STATUS");
  ttn.showStatus();

  pinMode(sensorPin, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);

}


/*
   LOOP
*/


void loop()
{
  debugSerial.println("-- LOOP");

  // If the user activated the sensor on the web plateform, switch the led on, read the sensor value and send the data
  // Else, switch the led off and keep the connection alive

  if (ONreceived) {
    readAndSend();
  } else {
    keepAlive();
  }
  
  delay(500);
}


/*
   FUNCTIONS
*/


/*
   Reads the sensor value and sends it to the Things Network
*/

void readAndSend() {

  // Read sensor
  int sensorValue = analogRead(sensorPin);
  debugSerial.println(sensorValue);

  // Prepare payload
  byte payload[2];
  payload[0] = sensorValue >> 8;
  payload[1] = sensorValue;

  // Send it off
  ttn.sendBytes(payload, sizeof(payload));
}

/*
   Keeps the connection alive by sending a non-meanigful payload
*/

void keepAlive() {

  // Prepare payload (default payload FF)
  byte payload[1];
  payload[0] = 0xFF;

  // Send it off on port 2 to indicate this not a sensor value
  ttn.sendBytes(payload, sizeof(payload), 2);
}


/*
    Receives 1-byte messages
    Activates or desactivates the sensor with the boolean ONreceived and switches the LED states accordingly
*/

void message(const byte *payload, size_t length, port_t port)
{
  debugSerial.println("-- MESSAGE");

  // Only handle messages of a single byte
  if (length != 1) {
    return;
  }

  if (payload[0] == 0)
  {
    debugSerial.println("received: off");
    ONreceived = false;
    digitalWrite(LED_PIN, LOW);
    delay(1000);
  }
  else if (payload[0] == 1)
  {
    debugSerial.println("received: on");
    ONreceived = true;
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
  }
}
