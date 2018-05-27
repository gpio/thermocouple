/******************************************************************
 * 
 * Mesure de la température des fours et envois au serveur mqtt
 * Composant thermocouple: MAX31855 
 * Carte WEMOS D1 mini http://www.wemos.cc/
 * 
 * Le traitement est réalisé par un serveur Node-red http://nodered.org/
 * 
 * 
 *  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
 *
 * Pour installer la bibliotheque MQTT: PubSubClient:
  - menu Croquis -> Inclure une bibliothèque -> Gérer les bibliothèques : installer PubSubClient
 *
 *****************************************************************/
 
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h" //configuration Wifi, MQTT

/**********Creation de la classe ESP8266 WiFiClient****************/

WiFiClient wifiClient;
PubSubClient client(wifiClient);

/****************************** Variables globales *****************/

// entrées sorties

#define MAXVCC  D4
#define MAXDO   D3
#define MAXCS   D2
#define MAXCLK  D1 

// initialisation du thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

String mac = WiFi.macAddress();


/*Initialisation*/
void setup() {
  pinMode(MAXVCC, OUTPUT); digitalWrite(MAXVCC, HIGH);

  Serial.begin(9600);
  Serial.println("Mesure de temperature MAX31855");
  // Connexion au point d'acces.
  Serial.println(); Serial.println();
  Serial.print("Connexion ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connexion WiFi etablie");
  Serial.print("Address IP : "); Serial.println(WiFi.localIP());
  Serial.print("Address MAC: "); Serial.println(mac);
  // Configuration mqtt
  client.setServer(MQTT_SERVER, MQTT_SERVERPORT);

  if (client.connect("arduinoClient", MQTT_USERNAME, MQTT_KEY)) {
    client.publish("boot", mac.c_str());
  }
}


/*Boucle principale*/
void loop() {
//  float c = thermocouple.readCelsius();
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
 
  JSONencoder["mac"] = mac;
  JSONencoder["type"] = "temperature";
//  JSONencoder["valeur"] = (isnan(c))? (-2) : (-1);
  JSONencoder["valeur"] = thermocouple.readCelsius();
//  JSONencoder["valeur_int"] = thermocouple.readInternal();
//  JSONencoder["erreur"] = thermocouple.readError();
  
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println(JSONmessageBuffer);
  
  if (client.connect("arduinoClient", MQTT_USERNAME, MQTT_KEY)) { 
    if (client.publish("temperatures", JSONmessageBuffer) == true) {
      Serial.println("message envoye");
    } else {
      Serial.println("Erreur message non envoye");
    }
  }
  
 //client.loop();
  delay(10000); //10 secondes
 // delay(20000); //20 sec
  //delay(1000); //1 sec
}


