float temp;
int tempRaw = 0, brightnessled = 0;
byte tempArray[1];
char tempchar[10];

//i2c comms
void getTemp()
{
  Wire.beginTransmission(0x18);
  Wire.write(0xDD);
  Wire.write(0x14);
  Wire.endTransmission();
  Wire.requestFrom(0x18,3);

  if(Wire.read()==0x63)
  {
  tempArray[0] = Wire.read(); //Receive MSB
  tempArray[1] = Wire.read(); //Receive LSB

  tempRaw = (tempArray[0] << 8);
  tempRaw = (tempRaw + tempArray[1]);

  temp = (tempRaw / 9.31);
  }
}

void mosfet0On()
{
  Wire.beginTransmission(0x18);
  Wire.write(0xDE);
  Wire.write(0x32);        
  Wire.endTransmission();
}

void mosfet0Off()
{
  Wire.beginTransmission(0x18);
  Wire.write(0xDE);
  Wire.write(0x33);        
  Wire.endTransmission();
}

void mosfet0brightness()
{
  Wire.beginTransmission(0x18);
  Wire.write(0xDE);
  Wire.write(0x34);   
  Wire.write(brightnessled);
  Wire.endTransmission();
}

void mosfet0breatheOn()
{
  Wire.beginTransmission(0x18);
  Wire.write(0xDE);
  Wire.write(0x35);        
  Wire.endTransmission();
}

void mosfet0breatheOff()
{
  Wire.beginTransmission(0x18);
  Wire.write(0xDE);
  Wire.write(0x36);        
  Wire.endTransmission();
}
