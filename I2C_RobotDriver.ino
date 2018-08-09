/*****************************************
 **LIBRARIES, DEFINE, CONSTS & VARIABLES***
 ******************************************/
#include <Wire.h>
#include "TFMini.h"
//#include <Servo.h>
#include <AFMotor.h>

#define FAST 255
#define MED
#define INDEXMAX 8
#define  TURNTIME 1000
#define  DRIVETIME 500
//#define VIEW3 3
//#define VIEW4 4
#define TRIG_PIN A2
#define ECHO_PIN A3


const int distLimit = 45;
const int Angle_Limit = 35;
const int innerAngle = 60;

int rate = 1;
unsigned long interval= 2000; 
unsigned long prevTime = 0; 
volatile bool updateAvaliable = false;
char serialIn;
bool flag = false;
bool backFlag = false;
//Servo servoY;

// **************declare all 4 motor******************
AF_DCMotor motor1(1, MOTOR12_64KHZ);
AF_DCMotor motor2(2, MOTOR12_64KHZ);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);
//*****************************************************


/*****************************************
 * ARRAY DATA STRUCTURE TO HOLD PARTITIONS
 ******************************************/

class RANGE
{
  public:
    double r = 0;
    bool dataFlag = false;
};

RANGE range[8];
int count = 0;


//*****************************************************

void setup() {
  
  Serial.begin(9600);

  //***************I2C SETUP*****************
  Wire.begin(5);                
  Wire.onReceive(receiveEvent); 
  
  //***************ULTRASONIC*****************
  pinMode(ECHO_PIN, INPUT); 
  pinMode(TRIG_PIN, OUTPUT);
  //***************MOTOR SETUP*****************
  // turn on motor
  motor1.setSpeed(180);
  motor2.setSpeed(180);
  motor3.setSpeed(180);
  motor4.setSpeed(180);

  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);

 // servoY.attach(9);
 //servoY.write(120);
  
  delay(500);
}



void loop() {
  unsigned long currTime = millis();
  bool badData = flags();

  if(backFlag == true)
  {
    Serial.println("###E-reverse#### ");
     moveBackward();
     delay(DRIVETIME);
     moveStop();
     delay(100);  
  }

  if ( range[INDEXMAX -1].r != 0 && updateAvaliable == true && currTime - prevTime > interval  && !badData && backFlag == false)
  {
    updateAvaliable = false;
    prevTime = currTime;
    Serial.println();
    Serial.println();

    /*
     * BACK IF
     */

   if (range[3].r <= distLimit && range[4].r <= distLimit)
    {
      Serial.println("******************");
        /*******
        * BACK UP
        **********/
      Serial.println("BACK UP");
      moveBackward();
      delay(DRIVETIME/2);
       moveStop();

       /*******
        * angle yo self
        */

if(range[0].r < range[7].r)
{
      //if right center partition has obsticle, assume left has best path
      //find best partition on left side.

      int partition = scanData(4);
      
      if (partition == 4)
      {
        Serial.println("~~~~20 degrees left~~~~");
        
        turnLeft();
        delay(250);
        moveStop();
      }
      
      else  if (partition == 5)
      {
        Serial.println("~~~40 degrees lefft~~~~");
        
        turnLeft();
        delay(500);
        moveStop();


      }
      else if (partition == 6)
      {
        Serial.println("~~~60 degrees left~~~");
        
        turnLeft();
        delay(750);
        moveStop();
      }
      else if (partition == 7)
      {
        Serial.println("~~~90 dregrees left~~~");
        turnLeft();
        delay(1000);
        moveStop();
      }

}

      if(range[0].r > range[7].r)
{
      //if right center partition has obsticle, assume left has best path
      //find best partition on left side.

      int partition = scanData(3);
      
      
      if (partition == 3)
      {
        Serial.println("~~~turn 20 dregrees right~~~");
        
        turnRight();
        delay(250);
        moveStop();
        delay(1000);
      }
      else  if (partition == 2)
      {
        Serial.println("~~~~40 degrees right~~~");
        
        turnRight();
        delay(500);
        moveStop();

      }
      else if (partition == 1)
      {
        Serial.println("~~~40 degrees right~~~");
        turnRight();
        delay(750);
        moveStop();
      }
      else if ( partition == 0)
      {
        Serial.println("~~~~80 degrees right~~~~");
        
        turnRight();
        delay(1000);
         moveStop();
      }

    }
    Serial.println("******************");
}


    /*
     * FRONT IF
     */
 else if (range[3].r > distLimit && range[4].r > distLimit)
    { 
      Serial.println("******************");  
      /*
       * ANGLE IF
       */
     if (range[5].r < innerAngle || range[6].r < innerAngle)
    {

      //if left center partition has obsticle, assume right has best path
      //find best partition on right side.

      int partition = scanData(3);

      if (partition == 3)
      {
        Serial.println("~~~~turn 20 dregrees right~~~~");
        
        turnRight();
        delay(250);
        moveStop();
      }
      else  if (partition == 2)
      {
        Serial.println("~~~~40 degrees right~~~~");
        
        turnRight();
        delay(500);
        moveStop();

      }
      else if (partition == 1)
      {
        Serial.println("~~~60 degrees right~~~");
        turnRight();
        delay(750);
        moveStop();
      }
      else if ( partition == 0)
      {
        Serial.println("~~~~80 degrees right~~~~");
        
        turnRight();
        delay(1000);
         moveStop();
      }
      
    } // end of if
    
    else if(range[1].r < innerAngle || range[2].r < innerAngle)
{
      //if right center partition has obsticle, assume left has best path
      //find best partition on left side.

      int partition = scanData(4);
      
      if (partition == 4)
      {
        Serial.println("~~~~20 degrees left~~~~~");
        
        turnLeft();
        delay(250);
        moveStop();
      }
      
      else  if (partition == 5)
      {
        Serial.println("~~~~~40 degrees left~~~~~");
        
        turnLeft();
        delay(500);
        moveStop();


      }
      else if (partition == 6)
      {
        Serial.println("~~~~~60 degrees left~~~~");
        
        turnLeft();
        delay(750);
        moveStop();
      }
      else if (partition == 7)
      {
        Serial.println("~~~~90 dregrees left~~~~~");
        turnLeft();
        delay(1000);
        moveStop();
      }
  
    
    }
    
      Serial.println("FORWARD");
      moveForward();
      delay(DRIVETIME);
      moveStop();
      delay(1000);
      Serial.println("******************");
    }

    
  
}
backFlag = false;

}
  



   




int scanData(int startIndex)
{
  int largest = 0;
  double lastData = 0;
  //int danger = 0;

  
  if ( startIndex == 4)
  {
    for (int i = startIndex; i < INDEXMAX; i++)
    {
      if (range[i].r > lastData)
      {
        lastData = range[i].r;
        largest = i;
      }
    }
  }
  else if (startIndex == 3)
  {
    for (int i = 0; i <= startIndex; i++)
    {
      if (range[i].r > lastData)
      {
        lastData = range[i].r;
        largest = i;
      }
    }
  }
  else if (startIndex == 0)
  {
        for (int i = 0; i < INDEXMAX; i++)
    {
      if (range[i].r > lastData)
      {
        lastData = range[i].r;
        largest = i;
      }
    }
  }
  return largest;


}


bool flags()
{
  for(int i = 0; i < INDEXMAX; i++)
  {
    if (range[i].dataFlag == false)
    {
      return true;
    }
  }
  return false;
}

void storeData(int currentIndex, double val)
{
  if (val > 0)
  {
    if(val <= Angle_Limit && (currentIndex != 3 || currentIndex != 4))
    {
      backFlag = true;
      moveStop();
    }
    if(val <= distLimit && (currentIndex == 3 || currentIndex == 4))
      {
        Serial.println("###E-break###");
        moveStop();
      }
  updateAvaliable = true;
  range[currentIndex].dataFlag = true;
  range[currentIndex].r = val;
  Serial.println("storing: " + String(val) + " into: " + currentIndex);
  }
  else 
  {
   updateAvaliable = false;
   range[currentIndex].dataFlag = false;
   Serial.println("XXXXXXXXXXXXXXXXXXX");
   Serial.println("cannot update: " + String(val) + " into "+ String(currentIndex));
    Serial.println("XXXXXXXXXXXXXXXXXXX");
  }
  
}

/*
 * Drive commands for robot;
 */
void moveForward()
{
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor4.run(BACKWARD);
  motor3.run(BACKWARD);
}

void moveBackward()
{

  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor4.run(FORWARD);
  motor3.run(FORWARD);
}
void turnLeft()
{
  motor1.run(BACKWARD);
  motor2.run(FORWARD);
  motor4.run(FORWARD);
  motor3.run(BACKWARD);
}
void turnRight()
{
  motor1.run(FORWARD);
  motor2.run(BACKWARD);
  motor4.run(BACKWARD);
  motor3.run(FORWARD);
}

void moveStop()
{
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor4.run(RELEASE);
  motor3.run(RELEASE);
}



/*
 * I2C function 
 */
void receiveEvent(int howMany) {
  String temp = "";
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
   // Serial.print(c);
    temp += c;

  }
  int x = Wire.read();    // receive byte as an integer

  // Convert String to int and double values.
  String tempIndex = temp.substring(0, 1);
  int tempindex2 = tempIndex.toInt();
  String tempVal = temp.substring(2, temp.length()-1);
  int tempVal2 = tempVal.toDouble();
  
  // store data into array
  storeData(tempindex2, tempVal2);

}

