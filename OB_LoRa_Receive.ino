#include <SPI.h>
#include <LoRa.h>
#include <Wire.h> 
#include <WiFi.h>
#include <PubSubClient.h>
#include "SSD1306.h"

// Paramètres LORA
#define SCK 5 // GPIO5 - SX1278's SCK
#define MISO 19 // GPIO19 - SX1278's MISO
#define MOSI 27 // GPIO27 - SX1278's MOSI
#define SS 18 // GPIO18 - SX1278's CS
#define RST 14 // GPIO14 - SX1278's RESET
#define DI0 26 // GPIO26 - SX1278's IRQ (interrupt request)
//#define BAND 868E6 // 915E6
const long freq = 868E6;
const int SF = 9;


String LoRaData;
String message ="";

//Paramètres WIFI
const char* ssid     = "XXXXX";
const char* password = "XXXXX";

//Paramètres MQTT
const char* mqttServer = "192.168.0.11";
const int mqttPort = 1883;
#define TOPIC "mailBox"
WiFiClient wifiClient;
PubSubClient client(wifiClient);




SSD1306 display (0x3c, 4, 15);
String rssi = "RSSI -";
String packSize = "-";
String packet;



void loraData () {
  display.clear ();
  display.setTextAlignment (TEXT_ALIGN_LEFT);
  display.setFont (ArialMT_Plain_16);
  display.drawString (0, 0, rssi);
  display.drawString (0, 17, "Received " + packSize + " bytes");
  //display.drawStringMaxWidth (0, 34, 128, packet);
  display.drawString (0, 34, message);
  display.display ();
}

void cbk (int packetSize) {
  packet = "";
  packSize = String (packetSize, DEC);
  for (int i = 0; i  < packetSize; i++)  { packet =+ (char) LoRa.read();}
  rssi = "RSSI " + String(LoRa.packetRssi (), DEC);
  loraData ();
}

void setup () {
  pinMode (16, OUTPUT);
  digitalWrite (16, LOW); // set GPIO16 low to reset OLED
  delay (50);
  digitalWrite (16, HIGH); // while OLED is running, GPIO16 must go high,
  
  Serial.begin (115200);
  
  // Connexion LORA
  while (! Serial);
  Serial.println ();
  Serial.println ("LoRa Receiver Callback");
  SPI.begin (SCK, MISO, MOSI, SS);
  LoRa.setPins (SS, RST, DI0);
  if (! LoRa.begin (freq)) {
    Serial.println ("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.setSpreadingFactor(SF);
  //LoRa.onReceive(cbk);
  //LoRa.receive ();
  Serial.println ("LoRa Started");
  
  delay(10);

    // Connexion WiFi
    Serial.print("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  // Connexion MQTT
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("Mailbox" )) {
      Serial.println("MQTT Started");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }
  client.publish("Mailbox", "Hello");


  
  display.init ();
  display.flipScreenVertically ();
  display.setFont (ArialMT_Plain_24);
  
  delay (1500);
}

void loop () {
  int packetSize = LoRa.parsePacket ();
  message ="";
  if (packetSize) {
    
    // received a packet
    
    Serial.print(" Received packet ");
    Serial.print(packetSize);
    Serial.print(" bytes '");
    
    
    //read packet
    while (LoRa.available()) {
      message = message + (char)LoRa.read();
      }

    Serial.print(message);
    String jsonString = message;

    //print RSSI of packet
    int rssi = LoRa.packetRssi();
    Serial.print("' with RSSI ");   
    Serial.println(rssi);

    //affichage OLED
    cbk (packetSize); 
    
    //Publication MQTT
    //client.publish("Mailbox", "toto");
    client.publish("Mailbox", jsonString.c_str());
    //Serial.println(jsonString.c_str());
  }
  delay (10);

}
