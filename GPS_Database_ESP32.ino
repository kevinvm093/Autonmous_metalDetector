//Firebase sketch with I2C from arduino?


#include <IOXhop_FirebaseESP32.h>
#include <WiFi.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "String.h"


#define FIRE_HOST ""
#define FIRE_AUTH ""


const char* ssid       = "";// WiFi network name
const char* password   = ""; // WiFi network password

//******************************************************************
//Credentials for Google GeoLocation API...
const char* Host = ""; 
String thisPage = "";
String key = "";
//******************************************************************

int status = WL_IDLE_STATUS;
String jsonString = "{\n";

double latitude    = 0.0;
double longitude   = 0.0;
double accuracy    = 0.0;
int more_text = 1; // set to 1 for more debug output
#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

bool record = false;

static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 64)
//#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


//******************************************************************


int state= HIGH;
int trigger = 23;
int reading;
int prev = LOW;

long t1 = 0;
long debounce = 200;
int i =0;
char temp[10] = "";
bool info = false;
void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigger, INPUT);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(".");
  }
  
  Serial.println(" CONNECTED");
  Serial.println(WiFi.localIP());
  
  //connect to database
  Serial.println("Connecting to database....");
  Firebase.begin(FIRE_HOST, FIRE_AUTH);
  Serial.println("******Connected to database******");
/*
 * I2C set up goes here once wifi module is connected to database
 */
 Serial.println("Intializing Wire connection.....");
 Wire.begin(23,22);                /* join i2c bus with address 8 */
  Serial.println("I2C conntection established.....");

  //******************************************************************


  //******************************************************************
}

String longitude2 = "WPB";
String latitude2;
String location; // for debugging purposes


void loop() 
{
    //******************************************************************
char bssid[6];
  DynamicJsonBuffer jsonBuffer;
  Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found...");

    if (more_text) {
      // Print out the formatted json...
      Serial.println("{");
      Serial.println("\"homeMobileCountryCode\": 234,");  // this is a real UK MCC
      Serial.println("\"homeMobileNetworkCode\": 27,");   // and a real UK MNC
      Serial.println("\"radioType\": \"gsm\",");          // for gsm
      Serial.println("\"carrier\": \"Vodafone\",");       // associated with Vodafone
      //Serial.println("\"cellTowers\": [");                // I'm not reporting any cell towers
      //Serial.println("],");
      Serial.println("\"wifiAccessPoints\": [");
      for (int i = 0; i < n; ++i)
      {
        Serial.println("{");
        Serial.print("\"macAddress\" : \"");
        Serial.print(WiFi.BSSIDstr(i));
        Serial.println("\",");
        Serial.print("\"signalStrength\": ");
        Serial.println(WiFi.RSSI(i));
        if (i < n - 1)
        {
          Serial.println("},");
        }
        else
        {
          Serial.println("}");
        }
      }
      Serial.println("]");
      Serial.println("}");
    }
    Serial.println(" ");
  }
  // now build the jsonString...
  jsonString = "{\n";
  jsonString += "\"homeMobileCountryCode\": 234,\n"; // this is a real UK MCC
  jsonString += "\"homeMobileNetworkCode\": 27,\n";  // and a real UK MNC
  jsonString += "\"radioType\": \"gsm\",\n";         // for gsm
  jsonString += "\"carrier\": \"Vodafone\",\n";      // associated with Vodafone
  jsonString += "\"wifiAccessPoints\": [\n";
  for (int j = 0; j < n; ++j)
  {
    jsonString += "{\n";
    jsonString += "\"macAddress\" : \"";
    jsonString += (WiFi.BSSIDstr(j));
    jsonString += "\",\n";
    jsonString += "\"signalStrength\": ";
    jsonString += WiFi.RSSI(j);
    jsonString += "\n";
    if (j < n - 1)
    {
      jsonString += "},\n";
    }
    else
    {
      jsonString += "}\n";
    }
  }
  jsonString += ("]\n");
  jsonString += ("}\n");
  //--------------------------------------------------------------------

  Serial.println("");

  WiFiClientSecure client;

  //Connect to the client and make the api call
  Serial.print("Requesting URL: ");
  Serial.println("https://" + (String)Host + thisPage + "AIzaSyCYNXIYINPmTNIdusMjJloS4_BXSOff1_g");
  Serial.println(" ");
  if (client.connect(Host, 443)) {
    Serial.println("Connected");
    client.println("POST " + thisPage + key + " HTTP/1.1");
    client.println("Host: " + (String)Host);
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.println("User-Agent: Arduino/1.0");
    client.print("Content-Length: ");
    client.println(jsonString.length());
    client.println();
    client.print(jsonString);
    delay(500);
  }

  //Read and parse all the lines of the reply from server
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (more_text) {
      Serial.print(line);
    }
    JsonObject& root = jsonBuffer.parseObject(line);
    if (root.success()) {
      latitude    = root["location"]["lat"];
      longitude   = root["location"]["lng"];
      accuracy   = root["accuracy"];
    }
  }

  Serial.println("closing connection");
  Serial.println();
  client.stop();

  Serial.print("Latitude = ");
  Serial.println(latitude, 6);
  Serial.print("Longitude = ");
  Serial.println(longitude, 6);
  Serial.print("Accuracy = ");
  Serial.println(accuracy);

  //******************************************************************
  
reading = digitalRead(trigger);

Wire.requestFrom(8,1);

while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
    record = c;
  }
  Serial.println("********************");
  Serial.println(record);
  Serial.println("********************");
  /*
   if (info == true)
    {
      record = true;
      info = 'N';
    }*/

if(record)
{
 // if (state == HIGH)
  //{
    Serial.println("Metal has been detected ... ");
    
  /*  Wire.requestFrom(8, 10); /* request & read data of size 13 from slave 
 while(Wire.available())
 {
    char c = Wire.read();
    temp[i] = c;
  Serial.print(c);
  Serial.println(temp); 
  i++;
 }
 i = 0; */
 
 Serial.println("");
 int num = Firebase.getInt("log");
 num++;
 String coords = "/maps/coords";
 coords += String(num);
 String coordsLat = coords;
 String coordsLng = coords;
 String coordsAcc = coords;
 String coordsID = coords;
 
 coordsID += "/ID";
 coordsLat += "/Latitude";
 coordsLng += "/Longitude";
 coordsAcc += "/Accuracy";
 
 Serial.println("Storing location in database .....");

 Firebase.set(coordsID,  String(num));
 Firebase.set(coordsLat, String(latitude,6));
 Firebase.set(coordsLng, String(longitude,6));
 Firebase.set(coordsAcc, String(accuracy));
 
 Firebase.set("log", num++);
 //Firebase.set(Firebase.push("latitude", latitude, 6),1);
 if (Firebase.failed()) // for debugging purposes ....
 {
    Serial.print("setting /location failed:");
    Serial.println(Firebase.error());
    return;
  }
   Serial.println("Datastored .....");
   record = false; 
 // }
 }
   else 
   {
    Serial.println("No metal detected.");
   }
   t1 = millis(); 

  record = false; 
  Serial.println("********************");
  Serial.print("record set to ");
  Serial.println(record);
  Serial.println("********************");
  prev = reading;
// Firebase.setString("Location", temp);

 delay(1000);
}






