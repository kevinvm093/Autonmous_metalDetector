// Metal detector
// Runs a pulse over the search loop in series with resistor
// Voltage over search loop spikes
// Through a diode this charges a capacitor
// Value of capacitor after series of pulses is read by ADC

// Metal objects near search loop change inductance.
// ADC reading depends on inductance.
// changes wrt long-running mean are indicated by LEDs
// LED1 indicates rise in inductance
// LED2 indicates fall in inductance
// the flash rate indicates how large the difference is

// wiring:
// 220Ohm resistor on D2
// 10-loop D=10cm seach loop between ground and resistor
// diode (-) on pin A0 and (+) on loop-resistor connection
// 10nF capacitor between A0 and ground
// LED1 in series with 220Ohm resistor on pin 8
// LED2 in series with 220Ohm resistor on pin 9

// First time, run with with serial print on and tune value of npulse
// to get capacitor reading between 200 and 300

#include <Wire.h>

const byte npulse = 3;
const bool sound = true;
const bool debug = true;

const byte pin_pulse=A0;
const byte pin_cap  =A1;
const byte pin_LED1 =12;
const byte pin_LED2 =11;
const byte pin_tone =10;

char on;
char objectFound;
bool recorded = false;


void setup() {
  if (debug) Serial.begin(9600);
  pinMode(pin_pulse, OUTPUT); 
  digitalWrite(pin_pulse, LOW);
  pinMode(pin_cap, INPUT);  
  pinMode(pin_LED1, OUTPUT);
  digitalWrite(pin_LED1, LOW);
  pinMode(pin_LED2, OUTPUT);
  digitalWrite(pin_LED2, LOW);
  if(sound)pinMode(pin_tone, OUTPUT);
  if(sound)digitalWrite(pin_tone, LOW);
  
  delay(500);

  Wire.begin(5);
  Wire.onReceive(receiveEvent); // used for when mega commands metal detector to switch once prompted by user. 
  Wire.onRequest(requestEvent); // ESP32 will constantly ask metal detector if metal has been detected
  
}

const int nmeas=256;  //measurements to take
long int sumsum=0; //running sum of 64 sums 
long int skip=0;   //number of skipped sums
long int diff=0;        //difference between sum and avgsum
long int flash_period=0;//period (in ms) 
long unsigned int prev_flash=0; //time stamp of previous flash

void loop() {
  
 Serial.print("value ");
 Serial.println(objectFound);

 if (recorded == true)
 {
  objectFound = 'N';
  recorded = false;
 }

while(on == 'H')
{
  Serial.print("value ");
  Serial.println(objectFound);
  int minval=1023;
  int maxval=0;
  
  //perform measurement
  long unsigned int sum=0;
  for (int imeas=0; imeas<nmeas+2; imeas++){
    //reset the capacitor
    pinMode(pin_cap,OUTPUT);
    digitalWrite(pin_cap,LOW);
    delayMicroseconds(20);
    pinMode(pin_cap,INPUT);
    //apply pulses
    for (int ipulse = 0; ipulse < npulse; ipulse++) {
      digitalWrite(pin_pulse,HIGH); //takes 3.5 microseconds
      delayMicroseconds(3);
      digitalWrite(pin_pulse,LOW);  //takes 3.5 microseconds
      delayMicroseconds(3);
    }
    //read the charge on the capacitor
    int val = analogRead(pin_cap); //takes 13x8=104 microseconds
    minval = min(val,minval);
    maxval = max(val,maxval);
    sum+=val;
 
    //determine if LEDs should be on or off
    long unsigned int timestamp=millis();
    byte ledstat=0;
    if (timestamp<prev_flash+10)
    {
      if (diff>0)ledstat=1;
      if (diff<0)ledstat=2;
    }
    if (timestamp>prev_flash+flash_period){
      if (diff>0)ledstat=1;
      if (diff<0)ledstat=2;
      prev_flash=timestamp; 
    }
    if (flash_period>1000)ledstat=0;

    //switch the LEDs to this setting
    if (ledstat==0){
      digitalWrite(pin_LED1,LOW);
      digitalWrite(pin_LED2,LOW);
      if(sound)noTone(pin_tone);
      objectFound = 'N';
    }
    if (ledstat==1){
      digitalWrite(pin_LED1,HIGH);
      digitalWrite(pin_LED2,LOW);
      if(sound)tone(pin_tone,2000);
      objectFound = 'Y'; 
    }
    if (ledstat==2){
      digitalWrite(pin_LED1,LOW);
      digitalWrite(pin_LED2,HIGH);
      if(sound)tone(pin_tone,500);
      objectFound = 'Y'; 
    }
  
  }

  //subtract minimum and maximum value to remove spikes
  sum-=minval; sum-=maxval;
  
  //process
  if (sumsum==0) sumsum=sum<<6; //set sumsum to expected value
  long int avgsum=(sumsum+32)>>6; 
  diff=sum-avgsum;
  if (abs(diff)<avgsum>>10){      //adjust for small changes
    sumsum=sumsum+sum-avgsum;
    skip=0;
  } else {
    skip++;
  }
  if (skip>64){     // break off in case of prolonged skipping
    sumsum=sum<<6;
    skip=0;
  }

  // one permille change = 2 ticks/s
  if (diff==0) flash_period=1000000;
  else flash_period=avgsum/(2*abs(diff));    
    
  if (debug){
    Serial.print(nmeas); 
    Serial.print(" ");
    Serial.print(minval); 
    Serial.print(" ");
    Serial.print(maxval); 
    Serial.print(" ");
    Serial.print(sum); 
    Serial.print(" ");
    Serial.print(avgsum); 
    Serial.print(" ");
    Serial.print(diff); 
    Serial.print(" ");
    Serial.print(flash_period); 
    Serial.println();
  }
   if (recorded == true)
 {
  objectFound = 'N';
  recorded = false;
 }
 Serial.println("********************");
  Serial.print("objectFound ==  ");
  Serial.println(objectFound);
  Serial.println("********************");
 objectFound = 'N';
  Serial.println("********************");
  Serial.print("recorded set to ");
  Serial.println(recorded);
  Serial.println("********************");
}
/*
if on == false metal detector will be off,
if on == true then metal detector will turn on. 
*/
}

void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  on = Wire.read();    // receive byte as an integer
  //Serial.println(howMany);         // print the integer
}

void requestEvent()
{
  Wire.write(objectFound);  /*send string on request */
 
  recorded = true;
}
