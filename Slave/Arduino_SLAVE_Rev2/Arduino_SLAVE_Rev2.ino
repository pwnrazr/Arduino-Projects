#include <Wire.h>
unsigned int temp, tempAvg;
byte tempArray[1], wirerecv[2];
unsigned int select = 0;

int mosfet0state = 0;
int mosfet0brightness = 0, mosfet0maxbrightness = 255;
int mosfet0breathe = 0;
int fadeamt = 1;

unsigned long previousMillis = 0, currentMillis;
unsigned long previousMillis2 = 0;
long interval = 6, interval2 = 30;

void setup() 
{
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
  
  tempAvg = 0;
  for(int i = 0; i < 50; i++)
  {
    tempAvg = tempAvg + analogRead(0); //Read temperature
  }
  temp = (tempAvg / 50);
  tempArray[0] = (temp >> 8); //MSB
  tempArray[1] = (temp);      //LSB

  mosfet0smooth();
  if(mosfet0breathe == 1)
  {
  mosfet0breatheMode();
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
    case 221: //temp control
    if(wirerecv[1]==20)
    {
      select = 1;
      break;
    }
    case 222: //Mosfet 0 Control
      if(wirerecv[1]==50) //On
      {
        mosfet0state = 1;
        //analogWrite(9, mosfet0maxbrightness); 
      }
      else if(wirerecv[1]==51) //Off
      {
        mosfet0state = 0;
        //analogWrite(9, 0);
      }
      else if(wirerecv[1]==52) //Brightness
      {
        mosfet0maxbrightness = wirerecv[2];
      }
      else if(wirerecv[1]==53) //Breathing mode enable
      {
        mosfet0breathe = 1;
      }
      else if(wirerecv[1]==54) //Breathing mode disable
      {
        mosfet0breathe = 0;
      }
      else
      {
        break;
      }
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

void mosfet0smooth()
{
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    if(mosfet0breathe == 0)
    {
      if(mosfet0brightness < mosfet0maxbrightness && mosfet0state==1)
      {
        analogWrite(9, mosfet0brightness);
        mosfet0brightness = mosfet0brightness + 1;
        //Serial.println(mosfet0brightness);
      }
      else if(mosfet0brightness > mosfet0maxbrightness && mosfet0state==1)
      {
        analogWrite(9, mosfet0brightness);
        mosfet0brightness = mosfet0brightness - 1;
        //Serial.println(mosfet0brightness);
      }
      if(mosfet0brightness==mosfet0maxbrightness)  //debug 
      {
        //Serial.println("ended smoothing");
      }

      if(mosfet0state==1)
      {
        if(mosfet0brightness < mosfet0maxbrightness)
        {
          analogWrite(9, mosfet0brightness);
          mosfet0brightness++;
        }
      }

      if(mosfet0state==0)
      {
        if(mosfet0brightness == 0)
        {
          analogWrite(9, 0);
        }
        if(mosfet0brightness > 0)
        {
          mosfet0brightness--;
          analogWrite(9, mosfet0brightness);
          //Serial.println(mosfet0brightness);
        }
      }
    }
  }
}

void mosfet0breatheMode()
{
  if (currentMillis - previousMillis2 >= interval2) 
  {
    previousMillis2 = currentMillis;
    //Serial.println(mosfet0brightness);

    if(mosfet0brightness <= 10)
    {
      mosfet0brightness = 10;
    }
    analogWrite(9, mosfet0brightness);

    mosfet0brightness = mosfet0brightness + fadeamt;

    if (mosfet0brightness <= 10 || mosfet0brightness >= 70) 
    {
      fadeamt = -fadeamt;
    }
  }
}


