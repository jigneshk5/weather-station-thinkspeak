#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <ESP8266HTTPClient.h> 
#define DHTTYPE DHT11   // DHT 11
//Replace your wifi credentials here
const char* ssid     = "Tenda_10DF10";//Replace with your Wifi Name
const char* password = "qwerty1478";// Replace with your wifi Password
String SERVER_IP= "api.thingspeak.com";
//change your channel number here
unsigned long myChannelNumber =1385103;//Replace with your own ThingSpeak Account Channle ID
const char * myWriteAPIKey= "0GHI8HD3L66L9LDS";
const char * myReadAPIKey= "1E6D83ZEF904GKQT";
//channel fields. 
unsigned int field1 = 1;
unsigned int field2 = 2;
unsigned int field4 = 4;
unsigned int field5 = 5;

WiFiClient  client;
HTTPClient http;
const int DHTPin = 5;   //D1
const int red = D7;     
const int green = D8; 
int redVal, greenVal;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(A0, INPUT);
  // We start by connecting to a WiFi network
  dht.begin();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);

}
void readChannel(int field){
  //get the last data of the field4
  int val = ThingSpeak.readFloatField(myChannelNumber, field);
  // Check the status of the read operation to see if it was successful
  int statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200 && field==field4){
    redVal= val;
    //Serial.println("Red Status: " + String(val));
    if(val>0){
      digitalWrite(red, HIGH);
    }else{
      digitalWrite(red, LOW);
    }
  }
  else if(statusCode == 200 && field==field5){
    greenVal= val;
    //Serial.println("Green Status: " + String(val));
    if(val>0){
      digitalWrite(green, HIGH);
    }else{
      digitalWrite(green, LOW);
    }
  }
  else{
    Serial.println("Problem reading field "+String(field)+". HTTP error code " + String(statusCode)); 
  }
}
void loop() {

 readChannel(field4);
 readChannel(field5);

  float h = dht.readHumidity();
  float temp = dht.readTemperature();
    if (isnan(h) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
   int ldr = analogRead(A0);
    if(ldr<100){
        sendEmail();
     }
  Serial.println("Temp: "+String(temp)+" , "+"Humidity: "+String(h)+" Brightness: "+String(ldr)+" Red: "+String(redVal)+" Green: "+String(greenVal));

  // set the fields with the values
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, ldr);
  ThingSpeak.setField(4, redVal);
  ThingSpeak.setField(5, greenVal);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel updated successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(20000);
 
}
void sendEmail(){
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://"+SERVER_IP+"/alerts/send"); //HTTP
    http.addHeader("Content-Type", "application/json");
    http.addHeader("ThingSpeak-Alerts-API-Key", "TAKTU4HFYRAG8WJJM8J0W");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST("{\"subject\":\"Alert from ThinkSpeak\", \"message\":\"LDR crosses the Threshold\"}");

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == 202) {
        const String& payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}
