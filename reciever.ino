// This will include the respective libraries depending upon the board chosen;
// ESP32 or ESP8266. Please ensure that you have them installed before compiling the code.

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

// Included in both cases.

#include <LoRa.h>
#include <SPI.h>

// Defining the pins of ESP32-30Pins to be connected to the SX1278 module
#define ss 5
#define rst 14
#define dio0 2

//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Replace them with your own WiFi credentials
#define WIFI_SSID "F A I S A L"
#define WIFI_PASSWORD "pakistan123"

// You don't have to temper with them
#define API_KEY "AIzaSyB7C2qdlmOx8Srxa4wmsFEVCT8yCCFArHs"
#define DATABASE_URL "esp32withlora-default-rtdb.firebaseio.com"
#define USER_EMAIL "sajitha00@gmail.com"
#define USER_PASSWORD "sajitha123"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

// Defining variables for string parsing
int ind1=0;
int ind2=0;
String cut1;
String cut2;

void setup() 
{
  // Change 115200 to 9600 if you don't see alien characters at the serial monitor
  Serial.begin(115200);
  
  while (!Serial);
  Serial.println("LoRa Receiver");

  // Connecting to WiFi according to the specifications given.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);

  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  // Setting pins for connection with the Lora Module
  LoRa.setPins(ss, rst, dio0);    // setup LoRa transceiver module
 
  // Setting Lora in 433MHz mode
  while (!LoRa.begin(433E6))     // 433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }

  // This is to define which radios to listen to, and which to ignore.
  // Should be the same for both the transmitter and the reciever.
  LoRa.setSyncWord(0xA5); //
  Serial.println("LoRa Initializing OK!");
}
 
// Run this loop forever
void loop() {

  String incomingPacket = "";
  // try to parse packet
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available()) {
      incomingPacket = incomingPacket+(char)LoRa.read();
    }
    
    Serial.println("The recieved packet is: ");
    Serial.println(incomingPacket);
    
    // Declaring empty strings for storing levels
    String levelCO= "";
    String levelDUST="";

    // Parsing the incoming string
    ind1 = incomingPacket.indexOf("|");  // indexing starts from zero
    ind2 = incomingPacket.indexOf("|",ind1+1);
    cut1 = incomingPacket.substring(ind1+1,ind2);   //captures the first string cut
    cut2 = incomingPacket.substring(ind2+1,incomingPacket.length());
    
    // Checking if the data came from TX1 or TX2 and performing the operations respectively
    if (incomingPacket.indexOf("1#x#")>0) {
      if ((cut1.toFloat())<800){
        levelCO="LOW";
      }
      else if ((cut1.toFloat())>800 && (cut1.toFloat())<1000){
        levelCO="MEDIUM";
      }
      else if ((cut1.toFloat())>1000){
        levelCO="HIGH";
      }

      if ((cut2.toFloat())<400){
        levelDUST="LOW";
      }
      else if ((cut2.toFloat())>400 && (cut2.toFloat())<700){
        levelDUST="MEDIUM";
      }
      else if ((cut2.toFloat())>700){
        levelDUST="HIGH";
      }

      // Writing corresoponding values to the database.
      Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, "/Tx1/CO_level", cut1.toFloat()) ? "CO reading of Road 1 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, "/Tx1/DUST_density", cut2.toFloat()) ? "DUST reading of Road 1 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.printf("Set float... %s\n", Firebase.setString(fbdo, "/Tx1/CO_level_L", levelCO) ? "CO level of Road 1 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.printf("Set float... %s\n", Firebase.setString(fbdo, "/Tx1/DUST_density_L", levelDUST) ? "DUST level of Road 1 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.println("Sensor readings of Road 1 were saved to Firebase!");
    }
    
    else if (incomingPacket.indexOf("2#x#")>0) {
      if ((cut1.toFloat())<800){
        levelCO="LOW";
      }
      else if ((cut1.toFloat())>800 && (cut1.toFloat())<1000){
        levelCO="MEDIUM";
      }
      else if ((cut1.toFloat())>1000){
        levelCO="HIGH";
      }

      if ((cut2.toFloat())<400){
        levelDUST="LOW";
      }
      else if ((cut2.toFloat())>400 && (cut2.toFloat())<700){
        levelDUST="MEDIUM";
      }
      else if ((cut2.toFloat())>700){
        levelDUST="HIGH";
      }
      Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, "/Tx2/CO_level", cut1.toFloat()) ? "CO reading of Road 2 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, "/Tx2/DUST_density", cut2.toFloat()) ? "DUST reading of Road 2 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.printf("Set float... %s\n", Firebase.setString(fbdo, "/Tx2/CO_level_L", levelCO) ? "CO level of Road 1 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.printf("Set float... %s\n", Firebase.setString(fbdo, "/Tx2/DUST_density_L", levelDUST) ? "DUST level of Road 1 was saved to Firebase!" : fbdo.errorReason().c_str());
      Serial.println("Sensor readings of Road 2 were saved to Firebase!");
    }
    else
    {
      Serial.println("Sensor readings weren't matched to be of either Road 1 or 2.");
    }

    // print RSSI of packet
    Serial.print("RSSI of last recieved Packet was: ");
    Serial.println(LoRa.packetRssi());
  }
}