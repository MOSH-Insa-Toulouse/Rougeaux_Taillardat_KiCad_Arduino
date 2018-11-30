
# LoRa Project: Smart Gas Sensor
> Elie Taillardat & Sophie Rougeaux

##### *Key-words:* ```Arduino``` ```KiCad``` ```The Thing Network``` ```LoRa``` ```Node-RED``` ```IoT``` ```Gas sensor```

This project is realised in the 5th year PTP ISS at INSA Toulouse.
A **LoRa-connected gas sensor** has been developed to alert the user about the concentration of health threatening gas near the sensor.

# Content

This repository contains :
- **KiCad PCB Files**: A shield for the gas sensor to plug on an Arduino Uno (made with [KiCad](http://kicad-pcb.org/))
- **Arduino Code**: The code for the microcontroler (using [Arduino IDE](https://www.arduino.cc/en/Main/Software))

A **Node-RED Dashboard** has been done to stay connected with your sensor. The development of this HMI is described at the end of this Readme. ([Node-RED](https://nodered.org/) and [Freeboard.io](https://freeboard.io/))


## KiCad PCB Files
### Overview
The shield is composed of :
- A home-made gas sensor (made at [AIME](https://www.aime-toulouse.fr/cmsms/index.php))
- An amplifier stage for home-made gas sensor
- A commercial gas sensor for the calibration of the home-made one (Grove connectors)
- A LoRa chip for the data transmission
- A LED as an on/off indicator

> We built our own footprints libraries expect for the Arduino Uno component

### Schematics

![](./assets/Schematic.PNG)

### PCB Layout

![](./assets/PCBLayout.PNG)

These parameters have been respected to respect the INSA PCB production capacity : 

 - Isolation : 0.4
 - Largeur piste : 0.8
 - Via : 1.6
 - Perçage Via : 0.8

### 3D Visualization

![](./assets/3D.JPG)

## Arduino Board & Code
We have done a **prototype** using a Grove gas sensor to measure the gas concentration in the air.

### Arduino Board
The elements in this project are : 

 - An Arduino Uno
 - A LoRa chip RN2783A
 - A Grove gas sensor (v1.4) MQ-9
 - A LED

![](./assets/arduino.jpg)

### Overview of the code
Data from a **Grove gas sensor** is transmitted with a **RN2483A chip**, via the **LoRa network**, to a TTN ([The Things Network](https://www.thethingsnetwork.org/)) account in which the Arduino device is registered.
The Arduino listens for incoming messages from a **dashboard** that is linked with the TTN account and allows the user to switch the gas sensor **on and off**.
A **LED** switches on and off according to the gas sensor state.

> You will find more information about the dashboard in the next section

#### Libraries
The communication bewteen the Arduino and the RN2483A is **serial**, so you will need the `SoftwareSerial` library. 
Communication with [The Things Network](https://www.thethingsnetwork.org/) is achieved with the very well written`TheThingsNetwork` library.

#### LoRa setup
To send data to [The Things Network](https://www.thethingsnetwork.org/), you will need to create an account, register your device, and enter your **device IDs** in the Arduino code : 
```c
// Set your AppEUI and AppKey
const char *devAddr = "26011253";
const char *nwkSKey = "D1A126A957FC1C25F20D3DB5379E3DE6";
const char *appSKey = "FC290B61D206CD05073F42CF673DE592";
```
The, the TTN library provides functions to quickly **setup your LoRa chip**, like `personalize(devAddr, nwkSKey, appSKey);`.

#### Sending sensor values
To send data, the function `readAndSend` reads the analog data from the sensor and prepares the **payload** containing the read value in **bytes**. Then, `sendBytes` function from TTN is used to transmit the data.
```c
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
```
#### Receiving data 
The **callback function** `onMessage` provided by the TTN library listens for an incoming message (**downlink mode** in TTN).
When a message is received, the `message` function is called.

If a `0` is received, the LED switches off and we will **stop** reading the gas sensor values. We will just keep the connection **alive**:
```c
void keepAlive() {

  // Prepare payload (default payload FF)
  byte payload[1];
  payload[0] = 0xFF;

  // Send it off on port 2 to indicate this not a sensor value
  ttn.sendBytes(payload, sizeof(payload), 2);
}
```
When a `1` is received, we switch on the light and we will **start** reading the sensor values using the previsouly presented `readAndSend` function.

## Node-Red Dashboard
### TTN data payload
Data sent by the **Arduino board** through **LoRa network** are decoded thanks to our ```Decoder``` function that we precised in the *Payload Format* section on TTN, which allows to generate an understandable payload with the bytes received, as ```json``` format for instance.
Our ```Decoder``` is presented below:
```javascript
function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  if (port === 2) { // send via the dashboard to stop gas sensor, see next chapter
    return { waiting: true };
    
  } else if (port === 1) {
    var gas = (bytes[0] << 8) | bytes[1];       // value between 0 and 1024
    var nano = (bytes[2] << 8) | bytes[3];
    
    // transform into readable data
    var gas_volt = gas / 1024 * 5.0;
    var gas_ratio = (5.0 - gas_volt)/gas_volt;  // see datasheet for the calculation
    var R0 = 9.3;                               // sensor resistance at 1000ppm LPG in the clean air
    return {
      gas: (gas_ratio/R0).toFixed(2),
      nano: nano
    };
  }
}
```
> We code the ```nano``` variable in prevision of the future nano-particules sensor that we will design at AIME). For now it is useless.

And the result for a simulated uplink payload on port 1: ``` 02 BE 01 F5 ```
```json
{
  "gas": 7.02,
  "nano": 5.01
}
```

### Node-RED
Once the payload is well interpreted in TTN, we use the tool **Node-RED** built on Node.js, to deal with our data and then display them in a dashboard for user consultation.

![](./assets/node-red.png)

We use a ``` ttn uplink ``` node linked to our application (```App ID: 20079652```) and registered device (```Device ID: arduino_gaz_sensor```). When a data is received, we can observe in the **debug** console the ```json``` payload received (output of the debug node, cf. picture above in the right panel). Therefore we received the payload as expected.
The dashboard compiled is shown below:

![](./assets/dashboard.gif)

As we can see, **we add a switch to control the state of the gas sensor remotely**.
When switched off for example, the payload sent in the ```ttn downlink``` node looks like:
```json
{
  "gas_sensor": false,
}
```

Then our Encoder on TTN sends to the Arduino board a bytes payload ```00``` or ```01``` (on port 2 or 3):
```javascript
function Encoder(object, port) {
  // Encode downlink messages sent as
  // object to an array or buffer of bytes.
  var bytes = [];
  if (port === 2) bytes[0] = object.gas_sensor ? 1 : 0;
  if (port === 3) bytes[0] = object.nano_sensor ? 2 : 0;

  return bytes;
}
```

When we simulate this behavior in real time, we can see the expected result on the TTN console output, and the LED switched off on our Arduino board:

![](./assets/ttn.png)

> Here, we can see that when switched off, the Arduino board stops sending data and is in a *waiting* mode. Once we switched on, the data are sent and received again.

### Freeboard.io
In addition to the localhost Node-RED dashboard we created, we prefered to use also Freeboard.io, to access the interface anywhere, and not only on our laptop localhost (http://127.0.0.1:1880/ui/).
That's the aim of the dweet node, which transfers the payload to a specific **topic**, that we will use in our freeboard dashboard.

**Dashboard link:
https://freeboard.io/board/Wauqxs**

> The curve allows the user to interpret the Gas ratio, and obtain a value in **ppm** of the gas concentration.

![](./assets/freeboard.gif)


## Contact
- Elie Taillardat: taillard@etud.insa-toulouse.fr
- Sophie Rougeaux: rougeaux@etud.insa-toulouse.fr
