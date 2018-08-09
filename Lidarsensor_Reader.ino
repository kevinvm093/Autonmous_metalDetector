#include <Wire.h>

const byte metalDetect = A1;
const byte LED1 = 12;
const int counter = 300;

bool flag = false;

void setup() 
{
  Serial.begin(9600);
  
  Wire.begin(8);
  Wire.onRequest(requestEvent); 
  
  pinMode(metalDetect, INPUT);
  pinMode(LED1, OUTPUT);
  
  delay(100);

}

void loop() 
{
  int count = 0;
  float val = 0;

for(int i = 0; i < counter; i++)
{
   val += analogRead(metalDetect); 
}


  val /= counter;
  Serial.println(val);
  
  if(val > 900)
  {
    Serial.println("metal found");
    digitalWrite(LED1, HIGH);
    flag = true;
  }
  delay(20);
  // digitalWrite(LED1, LOW);
  

}

void requestEvent() 
{
  Wire.write(flag); 
  flag = false;
  digitalWrite(LED1, LOW);
  delay(20);
}
