// Including the dependencies, Please ensure that you have installed them before compiling this code.
// Mostly they come pre-installed in the latest versions of Arduino IDE.

#include <LoRa.h>
#include <SPI.h>
#include <GP2Y1010AU0F.h>
 
// Defining the pins of ESP32-30Pins to be connected to the SX1278 module
#define ss 5
#define rst 14
#define dio0 2

// Defining the pins for connecting to the CO Sensor - MQ7
#define CO_pin 35

// Defining the pins for connecting to the DUST Sensor - Sharp GP2Y1010 AUF
#define DU_pin_MP 34
#define DU_pin_LED 32

int measurePin = DU_pin_MP;   // Connect dust sensor analog measure pin to Arduino A0 pin
int ledPin     = DU_pin_LED;    // Connect dust sensor LED pin to Arduino pin 2

GP2Y1010AU0F dustSensor(ledPin, measurePin); // Construct dust sensor global object

float dustDensity = 0;

float COsensorValue = 0;

int counter = 0;
 
void setup() 
{
  // Change 115200 to 9600 if you don't see alien characters at the serial monitor
  Serial.begin(115200);

  // For logging the setting up data on serial monitor
  while (!Serial);
  Serial.println("LoRa Sender");
  
  // Setting the pins for LoRa connection
  LoRa.setPins(ss, rst, dio0);    // setup LoRa transceiver module
  
  // Setting Lora in 433MHz mode
  while (!LoRa.begin(433E6))     // 433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }

  // This is to define which radios to listen to, and which to ignore.
  // Should be the same for both the transmitter and the reciever.
  LoRa.setSyncWord(0xA5);

  Serial.println("LoRa Initializing OK!");

  // Enabling the dust sensor
  dustSensor.begin();

  // Call this function when the transmission is done.
  LoRa.onTxDone(onTxDone);
}
 
void loop() 
{
  // Taking the CO value from the sensor and storing it in a variable.
  COsensorValue = analogRead(CO_pin);

   // Taking the DUST density value from the sensor in ()
  dustDensity = dustSensor.read();

  if (isnan (COsensorValue) || isnan (dustDensity)) {
    Serial.println ("Failed to read from one of the sensors!");
    return;
  }

  Serial.print("CO level in the Air: "); Serial.print(COsensorValue);
  String sendToFirebase = String ("2#x#") + String ("|") + String (COsensorValue,3) + String ("|");
  Serial.print ("Dust Density of Air: "); Serial.print (dustDensity);
  sendToFirebase = sendToFirebase + String (dustDensity,3); 

  // wait until the radio is ready to send a packet
  while (LoRa.beginPacket() == 0) {
    Serial.print("waiting for radio ... ");
    delay(100);
  }

  Serial.print("Sending packet: ");
  Serial.println(counter);
 
  LoRa.beginPacket();   //Send LoRa packet to receiver
  LoRa.print(sendToFirebase);
  LoRa.endPacket(true);
 
  delay(10000);
}

void onTxDone() {
  Serial.println("Data was sent successfully from Road 2!");
}