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
  client.setServer(MQTT_SERVER, MQTT_SERVERPORT);
  Serial.begin(9600);
   
  Serial.println(""); 
  Serial.println("Reveil");
  pinMode(MAXVCC, OUTPUT); digitalWrite(MAXVCC, HIGH);
  delay(100);


  //plusieurs mesures, on prend le plus haute
  double temp = 0;
  double temp_= 0;
  for(int i =0; i<10; i++){
    temp = thermocouple.readCelsius();
    if (temp_>temp){
      temp=temp_;
    }
    temp_=temp;
  }


  
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["mac"] = mac;
  JSONencoder["type"] = "temperature";
  JSONencoder["valeur"] = temp; // +5 ; //+offset de calibration

  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  int essais = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    essais++;
     Serial.print(".");
     if (essais>=10){
       Serial.println("Echec Connexion WiFi");
       ESP.deepSleep(20e6);
     }
  }
  Serial.println();
  Serial.println("Connexion WiFi etablie");
  Serial.print("Address IP : "); Serial.println(WiFi.localIP());
  Serial.print("Address MAC: "); Serial.println(mac);

  Serial.println(JSONmessageBuffer);
  if (client.connect("arduinoClient", MQTT_USERNAME, MQTT_KEY)) { 
    if (client.publish(TOPIC, JSONmessageBuffer) == true) {
      Serial.println("message envoye");
      delay(100); //pour le QoS 2 
    }
  }
  Serial.println("Mise en veille");
  ESP.deepSleep(58e6); 

}

void loop() {
}

