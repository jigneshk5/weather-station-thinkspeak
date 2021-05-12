#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include "DHT.h"
#include "FirebaseJson.h"

#define WIFI_SSID "Tenda_10DF10"   
#define WIFI_PASSWORD "qwerty1478"

String SERVER_IP= "api.thingspeak.com";
const char serverAddress[] = "api.thingspeak.com";  // server address
int port = 80;
unsigned long lastMillis = 0;
#define DHTTYPE DHT11   // DHT 11
FirebaseJsonData jsonData;
FirebaseJson json;

const int DHTPin = 5;   //D1

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
WiFiClient client;
HTTPClient http;

const int red = D7;     
const int green = D8;     
int redVal, greenVal;

void setup()
{

  Serial.begin(115200);

  pinMode(red,OUTPUT);
  pinMode(green,OUTPUT);
  pinMode(A0,INPUT);
  dht.begin();
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

}

void loop() {
  
    Serial.println("making GET request");
    if (http.begin(client, "http://"+SERVER_IP+"/channels/1377195/feeds.json?results=1")) {  // HTTP

    // start connection and send HTTP header
    int httpCode = http.GET();
    if(httpCode==200){
      String response = http.getString();
      json.setJsonData(response);

      Serial.println(response);
      
      json.get(jsonData, "feeds/field4");
      //Serial.println(json);
      redVal= jsonData.intValue;
      if (jsonData.intValue>0)
        {
          digitalWrite(red,HIGH);
        }
       else{
          digitalWrite(red,LOW); 
       }
  
       json.get(jsonData, "feeds/field5");
      //Serial.println("Green Status: "+String(jsonData.intValue));
      greenVal= jsonData.intValue;
      if (jsonData.intValue>0)
        {
          digitalWrite(green,HIGH);
        }
       else{
          digitalWrite(green,LOW); 
       }
    }
  }
  
    float h = dht.readHumidity();
    float temp = dht.readTemperature();
      if (isnan(h) || isnan(temp)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }
     int ldr= analogRead(A0);
     if(ldr<100){
        sendEmail();
     }
      Serial.println("Temp: "+String(temp)+" , "+"Humidity: "+String(h)+" Brightness: "+String(ldr)+" Red: "+String(redVal)+" Green: "+String(greenVal));
    
      // assemble the body of the POST message:
      String postData = "{\"field1\":"+String(temp)+", \"field2\":"+String(h)+", \"field3\":"+String(ldr)+", \"field4\":"+String(redVal)+", \"field5\":"+String(greenVal)+"}";
    
      Serial.println("making POST request");

    http.begin(client, "http://"+SERVER_IP+"/update?api_key=RNHM43R7HH0XCCOF"); //HTTP
    http.addHeader("Content-Type", "application/json");

    // start connection and send HTTP header and body
    int httpCode = http.POST(postData);

    // httpCode will be negative on error
    if (httpCode > 0) {
      Serial.println("done");
    }

  delay(15000);
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
      if (httpCode == HTTP_CODE_OK) {
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
