
# LoRa Project: Smart Gas Sensor
> Elie Taillardat & Sophie Rougeaux

This project is realized in the 5th year PTP ISS at INSA Toulouse.
A **LoRa-connected gas sensor** has been developed to alert the user about the concentration of health threatening gas near the sensor.

# Content

This folder contains :
- **PCB Files**: A shield for the gas sensor to plug on an Arduino Uno (made with [KiCad](http://kicad-pcb.org/))
- **Arduino Code**: The code for the microcontroler, using [Arduino IDE](https://www.arduino.cc/en/Main/Software)
- **Dashboard Link**: The HMI, to stay connected with your sensor ([Node-RED](https://nodered.org/) and [Freeboard.io](https://freeboard.io/))


## PCB Files
### Overview
The shield is composed of :
- A home-made gas sensor (made at [AIME](https://www.aime-toulouse.fr/cmsms/index.php))
- A amplifier stage for home-made gas sensor
- A commercial gas sensor for the calibration of the home-made one (Grove connectors)
- The LoRa chip for the data transmission
- A LED as a threshold exceeded indicator

> We built our own footprints libraries expect for the Arduino Uno component

### Layout
These parameters have been respected to respect the INSA PCB production capacity :
|Parameters|Value|
|--|--|
|Isolation|0.4|
| Largeur piste | 0.8 |
|Diamètre Via|1.6|
|Perçage Via|0.8|

### 3D Model
> KiCad-Pcbnew-Affichage-3D Visualizer

## Node-Red Dashboard & Freeboard.io
### TTN data receiving
Data sent by the Arduino board through LoRA network are decoded thanks to a ```Decoder``` function that we precised in the *Payload Format* section, which allows to generate an understable payload with the bytes received, as ```json``` format for example.
Our ```Decoder``` is presented below:
```javascript
function Decoder(bytes) {
  // Decode an uplink message from a buffer
  var gaz = (bytes[0] << 8) | bytes[1];
  var nano = (bytes[2] << 8) | bytes[3];
  return {
    gaz: gaz / 100.0, //TODO: adapt data transformation
    nano: nano / 100.0
  };
}
```
And the result for a test payload: ``` 02 BE 01 F5 ```
```json
{
  "gaz": 7.02,
  "nano": 5.01
}
```

### Node-RED
Once the payload is well interpreted in TTN, we use the tool **Node-RED** built on Node.js, to deal with our datas and then display them in a dashboard for user consultation.
![](./assets/red-node.png)

We use a ``` ttn ``` node linked to our application and registered device. When a data is received, we can observe in the debug console the following result (output of the debug node):
![](./assets/debug-node-red.png)

Therefore we received the json payload as expected. Instead of using the dashboard provided by Node-RED directly, we prefered to use Freeboard.io, to access the interface anywhere, and not only on our laptop localhost.
That's the aim of the dweet node, which transfer the payload to a specific topic, that we will use in our freeboard dashboard.

### Freeboard.io
