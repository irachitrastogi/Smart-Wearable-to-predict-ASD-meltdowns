#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include "certs.h"
#include "ota.h"


const int oneWireBus = 4; 

OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

const char* ssid = "ssid";
const char* password = "pass";

const char* mqtt_server = "cs7ns6.ddns.net";
const uint32_t MQTT_PORT = 8883;
const char* MQTT_USER = "user";
const char* MQTT_PASSWD = "pass";

int RED_LED = 12; 
int GREEN_LED = 14; 

WiFiClientSecure secureClient = WiFiClientSecure();

//WiFiClient espClient;
PubSubClient client(secureClient);
long lastMsg = 0;

// LED Pin
const int ledPin = 0;

int pulsePin = 34;                 // Pulse Sensor purple wire connected to analog pin 34 , ADC6
// Volatile Variables, used in the interrupt service routine!
volatile int BPM = 75;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED,LOW);
  digitalWrite(GREEN_LED,HIGH);
  Serial.begin(115200);
  sensors.begin();
  ConnectWifi();
  client.setServer(mqtt_server, MQTT_PORT);
  secureClient.setCACert(CERT_CA);
  secureClient.setCertificate(CERT_CRT);
  secureClient.setPrivateKey(CERT_PRIVATE);

  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String payload_buff;
  for (int i=0;i<length;i++) {
    payload_buff = payload_buff+String((char)payload[i]);
  }
  Serial.println(payload_buff);
  if(payload_buff == "{\"alert\": true}"){
  digitalWrite(RED_LED,HIGH); 
  digitalWrite(GREEN_LED,LOW);
  }
  else{
    digitalWrite(RED_LED,LOW); 
  digitalWrite(GREEN_LED,HIGH);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client",MQTT_USER,MQTT_PASSWD)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("/cs7ns2/esp32/alerts/esp32-sm-01");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  server.handleClient();
  delay(1);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (QS == true) {    // A Heartbeat Was Found
    Serial.println("BPM = " + BPM);
    QS = false;                      // reset the Quantified Self flag for next time
  }
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    sensors.requestTemperatures(); 
    float temperatureF = sensors.getTempFByIndex(0) +12;
    Serial.println("TEMPERATIRE = " + String(temperatureF));
    StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["temperature"] = temperatureF;
  JSONencoder["heartrate"] = BPM;
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
  client.publish("/cs7ns2/esp32/sensors/esp32-sm-01", JSONmessageBuffer);
  }
}

void ConnectWifi(){

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  InitOta();
  }
