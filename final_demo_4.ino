
#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>
 
String WRITEAPIKEY = "1111";
String CHANNELID ="1020";
const char* ssid = "";
const char* password = "" ;
 
WiFiClient client;

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include<SoftwareSerial.h>

#define PRE_PIN          D5
#define VNOX_PIN         A0
#define VRED_PIN         A1
#define PRE_HEAT_SECONDS 10



int vnox_value = 0;
int vred_value = 0;
float sensorValue;
int RL;
float R0;
int Rs;
float ratio;
float ratio1;
int CO2ppm;
float Vout;

Adafruit_ADS1115 ads(0x48);

void setup() 
{
  ads.begin();
  pinMode(PRE_PIN, OUTPUT);
  Serial.println("MiCS-4514 Test Read");
  Serial.print("Preheating...");

  // Wait for preheating//
  digitalWrite(PRE_PIN, 1);
  delay(PRE_HEAT_SECONDS * 1000);
  digitalWrite(PRE_PIN, 0);
  Serial.println("Done");

  Serial.begin(115200);
  Wire.begin(D4, D3);
   WiFi.disconnect();
  delay(10);
  WiFi.begin(ssid, password);
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("NodeMcu connected to wifi...");
  Serial.println(ssid);
  Serial.println();
  delay(3000);
  Serial.println("");
  
 
}



float gettemperature()
{
int analogValue = ads.readADC_SingleEnded(0);
float volt = (analogValue*0.0625);
float millivolts = (volt*3300.0) / 1024.0; //5000 is the voltage provided by NodeMCU
float celsius = millivolts/10;
Serial.print("in DegreeC=   ");
Serial.println(celsius);

delay(2000);
return celsius;
}


void loop() 
{

  
  float temp = gettemperature();                        //To properly caculate relative humidity, we need the temperature. 
  float relativeHumidity = getHumidity(temp);
  Serial.println("Humidity :- ");
  Serial.print(relativeHumidity);
  Serial.println("%"); 

  delay(2000); //just here to slow it down so you can read it

  // Read analog values, print them out, and wait
  float supplyVolt=3.3;
  float vnox_value = ads.readADC_SingleEnded(3);
  float vnox = (vnox_value*0.0625);
  float vred_value = ads.readADC_SingleEnded(2);
  float vred = (vred_value*0.0625);
  float vn=(vnox/1024)*3300;
  float vco=(vred/1024)*3300;
  Serial.print(" vnox_value: ");
  Serial.println(vn);
  Serial.print(" vco_value: ");
  Serial.println(vco);
  delay(2000);
  Serial.println("");



  sensorValue = analogRead(A0);                               // MQ-135 part
  RL=10;
  Vout = sensorValue * 3.3 / 4095.0;
  //Rs = RL * (5 - Vout) / Vout;
  float Rs=((1023/sensorValue)-1)*RL;
  R0 = 76.63;
  ratio=Rs/R0;
  ratio1=ratio*0.3611;

  CO2ppm= (146.15*(2.868-ratio1)+10);
  Serial.println("CO2_ppm");
  Serial.print(CO2ppm); // prints the value of CO2
  delay(2000); // wait 2s for next reading

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
    HTTPClient http;  //Declare an object of class HTTPClient
    String getURL="http://aqmapi.herokuapp.com/api/data/add/";

    String fullURL=getURL+CHANNELID+"/"+WRITEAPIKEY+"/"+"temperature="+String(temp)+"&humidity="+String(relativeHumidity)+"&co2="+String(CO2ppm)+"&no2="+String(vn)+"&co="+String(vco);
    Serial.print("");
    Serial.println(fullURL);
    http.begin(fullURL);
    http.addHeader("Content-Type", "text/html");
    int httpCode = http.GET();                                  
    if (httpCode > 0) 
    { 
      String payload = http.getString();  
      Serial.println(payload);            
    }
    http.end();   //Close connection
  }
 
  delay(2000);    //Send a request every 2 seconds

}


float getHumidity(float degreesCelsius)               //humidity part
{
  float supplyVolt = 3.3;

  // read the value from the sensor:
  int  HIH4030_Value = ads.readADC_SingleEnded(1);
  float volt = (HIH4030_Value*  0.0625);
  
  
  float voltage =  volt / 1023. * supplyVolt; // convert to voltage value
  float sensorRH = 161.0 * voltage / supplyVolt - 25.8;
  float trueRH = sensorRH / (1.0546 - 0.0026 *degreesCelsius );             //temperature adjustment

  return trueRH;
}
