#include <Wire.h>
unsigned int temp;
byte tempArray[1], wirerecv[2];
unsigned int select = 0;
int fadeoffdone = 1,fadeondone = 1, smoothen = 0;
int brightness = 0, maxbrightness, fadeamt = 5;
unsigned long previousMillis = 0, currentMillis;
long interval = 30, interval2 = 6;

void setup() {
  Serial.begin(9600);
  Serial.println("Nano boot");
  Wire.begin(0x18); 
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  digitalWrite(SDA, LOW);  //Disables internal pull-up on i2c lines
  digitalWrite(SCL, LOW);  //For NodeMCU safety!

  pinMode(9 , OUTPUT);

  analogReference(INTERNAL);
}

void loop() //Polling
{
  currentMillis = millis();
  
  temp = analogRead(0); //Read temperature
  tempArray[0] = (temp >> 8); //MSB
  tempArray[1] = (temp);      //LSB

  if(fadeoffdone==0)
  {
    fadeoffled();
  }

  if(fadeondone==0)
  {
    fadeonled();
  }

  if(smoothen==1)
  {
    smoothled();
  }
}

void receiveEvent(int numBytes) //When Written to
{
  for(int i = 0; Wire.available() > 0; i++)
  {
    wirerecv[i] = Wire.read();
  }
  
  switch(wirerecv[0])
  {
/* deprecated
    case 1: //Select temperature read
      select = 1;
      break;
    case 252:
      maxbrightness = wirerecv[1];
      smoothen = 1;
      //brightness = wirerecv[1];
      //analogWrite(9, wirerecv[1]);
      Serial.println(maxbrightness);
      break;
    case 253: //sleep led
      fadeoffdone = 0;
      fadeondone=1;
      fadeamt = 1;
      interval = 100;
      break;
    case 254: //off desk led
      fadeoffdone = 0;
      fadeondone=1;
      fadeamt = 1;
      interval = 6;
      break;
    case 255: //on Desk LED
      fadeondone = 0;
      fadeoffdone=1;
      fadeamt = 1;
      interval = 6;
      break; */
//start edit
    case 221: //temp control
    if(wirerecv[1]==20)
    {
      select = 1;
      break;
    }
    case 222: //LED control
      if(wirerecv[1]==50) //on command
      {
        fadeondone = 0;
        fadeoffdone=1;
        fadeamt = 1;
        interval = 6;
      }
      else if(wirerecv[1]==51) //off command
      {
        fadeoffdone = 0;
        fadeondone=1;
        fadeamt = 1;
        interval = 6;
      }
      else if(wirerecv[1]==52) //sleep command
      {
        fadeoffdone = 0;
        fadeondone=1;
        fadeamt = 1;
        interval = 100;        
      }
      else if(wirerecv[1]==53) //brightness command
      {
        maxbrightness = wirerecv[2];
        smoothen = 1;
        //brightness = wirerecv[1];
        //analogWrite(9, wirerecv[1]);
        Serial.println(maxbrightness);  
      }
      else
      {
        break;  //end LED control
      }
    //end edit
  }
}

void requestEvent()
{
  switch(select)
  {
    case 1: //Returns temperature reading
      Wire.write(0x63);
      Wire.write(tempArray, 2); //send temp as 2 bytes
      break;
  }
}

void fadeoffled()
{
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    if(brightness > 0)
    {
      analogWrite(9, brightness);
      brightness = brightness - fadeamt;
      //Serial.println(brightness);
    }
    else if(brightness<=0)
    {
      brightness=0;
      digitalWrite(9, LOW);
      fadeoffdone=1;
    }
  //delay(30);
  }
}

void fadeonled()
{
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    if(brightness <= maxbrightness)
    {
      analogWrite(9, brightness);
      brightness = brightness + fadeamt;
      //Serial.println(brightness);
    }
    else if(brightness>=maxbrightness)
    {
      brightness=maxbrightness;
      analogWrite(9, brightness);
      fadeondone=1;
    }
  //delay(30);
  }
  //Serial.println(brightness);
}

void smoothled()
{
  if (currentMillis - previousMillis >= interval2) 
  {
    previousMillis = currentMillis;
    if(brightness < maxbrightness)
    {
      analogWrite(9, brightness);
      brightness = brightness + 1;
      //Serial.println(brightness);
    }
    else if(brightness > maxbrightness)
    {
      analogWrite(9, brightness);
      brightness = brightness - 1;
      //Serial.println(brightness);
    }
    if(brightness==maxbrightness)
    {
      smoothen=0;
      //Serial.println("ended smoothing");
    }
  }
}

