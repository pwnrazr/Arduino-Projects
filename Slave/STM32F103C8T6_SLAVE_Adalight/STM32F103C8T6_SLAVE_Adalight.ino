#include "serialFunc.h"

// --- General Settings
static const uint16_t 
  Num_Leds   =  99;        // strip length
static const uint8_t
  Brightness =  255;       // maximum brightness

// --- FastLED Setings
#define LED_TYPE     WS2812B // led strip type for FastLED
#define COLOR_ORDER  GRB     // color order for bitbang
#define PIN_DATA     PA8       // led data output pin
//#define PIN_CLOCK  7       // led data clock pin (uncomment if you're using a 4-wire LED type)

// --- Serial Settings
static const unsigned long
  SerialSpeed    = 115200; // serial port speed
static const uint16_t
  SerialTimeout  = 60;     // time before LEDs are shut off if no data (in seconds), 0 to disable

// --- Optional Settings (uncomment to add)
#define SERIAL_FLUSH         // Serial buffer cleared on LED latch
//#define CLEAR_ON_START     // LEDs are cleared on reset
//#define GROUND_PIN 10      // additional grounding pin (optional)
//#define CALIBRATE          // sets all LEDs to the color of the first

// --- Debug Settings (uncomment to add)
//#define DEBUG_LED 13       // toggles the Arduino's built-in LED on header match
//#define DEBUG_FPS 8        // enables a pulse on LED latch

// --------------------------------------------------------------------

#include <FastLED.h>
CRGB leds[Num_Leds];
uint8_t * ledsRaw = (uint8_t *)leds;

// A 'magic word' (along with LED count & checksum) precedes each block
// of LED data; this assists the microcontroller in syncing up with the
// host-side software and properly issuing the latch (host I/O is
// likely buffered, making usleep() unreliable for latch). You may see
// an initial glitchy frame or two until the two come into alignment.
// The magic word can be whatever sequence you like, but each character
// should be unique, and frequent pixel values like 0 and 255 are
// avoided -- fewer false positives. The host software will need to
// generate a compatible header: immediately following the magic word
// are three bytes: a 16-bit count of the number of LEDs (high byte
// first) followed by a simple checksum value (high byte XOR low byte
// XOR 0x55). LED data follows, 3 bytes per LED, in order R, G, B,
// where 0 = off and 255 = max brightness.

static const uint8_t magic[] = {
  'A','d','a'};
#define MAGICSIZE  sizeof(magic)

// Check values are header byte # - 1, as they are indexed from 0
#define HICHECK    (MAGICSIZE)
#define LOCHECK    (MAGICSIZE + 1)
#define CHECKSUM   (MAGICSIZE + 2)

enum processModes_t {Header, Data} mode = Header;

static int16_t
  c;
static uint16_t
  outPos;
static uint32_t
  bytesRemaining;
static unsigned long
  t,
  lastByteTime,
  lastAckTime;

// Debug macros initialized
#ifdef DEBUG_LED
  #define ON  1
  #define OFF 0

  #define D_LED(x) do {digitalWrite(DEBUG_LED, x);} while(0)
#else
  #define D_LED(x)
#endif

#ifdef DEBUG_FPS
  #define D_FPS do {digitalWrite(DEBUG_FPS, HIGH); digitalWrite(DEBUG_FPS, LOW);} while (0)
#else
  #define D_FPS
#endif

//My defines
unsigned char SerRecv[10];
bool ledstate = true;
unsigned int lastMode = 0;
unsigned int curMode = 0;   //defaults to adalight

unsigned int ledR;
unsigned int ledG;
unsigned int ledB;

void setup(){
  #ifdef GROUND_PIN
    pinMode(GROUND_PIN, OUTPUT);
    digitalWrite(GROUND_PIN, LOW);
  #endif

  #ifdef DEBUG_LED
    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(DEBUG_LED, LOW);
  #endif

  #ifdef DEBUG_FPS
    pinMode(DEBUG_FPS, OUTPUT);
  #endif

  #ifdef PIN_CLOCK
    FastLED.addLeds<LED_TYPE, PIN_DATA, PIN_CLOCK, COLOR_ORDER>(leds, Num_Leds);
  #else
    FastLED.addLeds<LED_TYPE, PIN_DATA, COLOR_ORDER>(leds, Num_Leds);
  #endif
  
  FastLED.setBrightness(Brightness);

  #ifdef CLEAR_ON_START
    FastLED.show();
  #endif

  Serial.begin(SerialSpeed);
  Serial.print("Ada\n"); // Send ACK string to host

  lastByteTime = lastAckTime = millis(); // Set initial counters

  //my stuff
  Serial1.begin(115200);
}

void loop()
{
  recvWithStartEndMarkers();
  if (newData == true) {
      strcpy(tempChars, receivedChars);
          // this temporary copy is necessary to protect the original data
          //   because strtok() used in parseData() replaces the commas with \0
      parseData();

      if(messagestr == "state")
      {
        if(integerRecv == 1)
        {
          LEDS.showColor(CRGB(0, 255, 0));
        }
        else if(integerRecv == 0)
        {
          LEDS.showColor(CRGB(255, 0, 0));
        }
      }
      newData = false;
  }
  
  switch(SerRecv[0])
  {
    case 1:
      curMode = 1;
    break;

    case 2:
      curMode = 2;
    break;

    case 3:
      curMode = 3;
    break;

    case 4:
      curMode = 4;
    break;

    case 5:
      curMode = 5;
    break;
    
    case 128:
      FastLED.setBrightness(SerRecv[1]);
      //Serial.println(SerRecv[1]);
    break;

    case 200:
      ledR = (SerRecv[1]);
    break;
    
    case 201:
      ledG = (SerRecv[1]);
    break;

    case 202:
      ledB = (SerRecv[1]);
    break;
    
    case 254: //Off led  
      ledstate = false;
    break;

    case 255: //On led
      ledstate = true;
      //curMode = lastMode;
    break;
  }
  SerRecv[0] = 0; //clears serial receive

  if(ledstate == true)
  {
    switch(curMode)
    {
      case 1:
        adalight();
        lastMode = curMode;
      break;
  
      case 2: //RGB custom
        LEDS.showColor(CRGB(ledR, ledG, ledB));
        lastMode = curMode;
      break;
  
      case 3:

        lastMode = curMode;
      break;
  
      case 4:

        lastMode = curMode;
      break;
  
      case 5:

        lastMode = curMode;
      break;
    }
  }
  else if(ledstate == false)
  {
    LEDS.showColor(CRGB(0, 0, 0));
  }
}

void adalight(){ 
  t = millis(); // Save current time

  // If there is new serial data
  if((c = Serial.read()) >= 0){
    lastByteTime = lastAckTime = t; // Reset timeout counters

    switch(mode) {
      case Header:
        headerMode();
        break;
      case Data:
        dataMode();
        break;
    }
  }
  else {
    // No new data
    timeouts();
  }
}

void headerMode(){
  static uint8_t
    headPos,
    hi, lo, chk;

  if(headPos < MAGICSIZE){
    // Check if magic word matches
    if(c == magic[headPos]) {headPos++;}
    else {headPos = 0;}
  }
  else{
    // Magic word matches! Now verify checksum
    switch(headPos){
      case HICHECK:
        hi = c;
        headPos++;
        break;
      case LOCHECK:
        lo = c;
        headPos++;
        break;
      case CHECKSUM:
        chk = c;
        if(chk == (hi ^ lo ^ 0x55)) {
          // Checksum looks valid. Get 16-bit LED count, add 1
          // (# LEDs is always > 0) and multiply by 3 for R,G,B.
          D_LED(ON);
          bytesRemaining = 3L * (256L * (long)hi + (long)lo + 1L);
          outPos = 0;
          memset(leds, 0, Num_Leds * sizeof(struct CRGB));
          mode = Data; // Proceed to latch wait mode
        }
        headPos = 0; // Reset header position regardless of checksum result
        break;
    }
  }
}

void dataMode(){
  // If LED data is not full
  if (outPos < sizeof(leds)){
    dataSet();
  }
  bytesRemaining--;
 
  if(bytesRemaining == 0) {
    // End of data -- issue latch:
    mode = Header; // Begin next header search
    FastLED.show();
    D_FPS;
    D_LED(OFF);
    #ifdef SERIAL_FLUSH
      serialFlush();
    #endif
  }
}

void dataSet(){
  #ifdef CALIBRATE
    if(outPos < 3)
      ledsRaw[outPos++] = c;
    else{
      ledsRaw[outPos] = ledsRaw[outPos%3]; // Sets RGB data to first LED color
      outPos++;
    }
  #else
    ledsRaw[outPos++] = c; // Issue next byte
  #endif
}

void timeouts(){
  // No data received. If this persists, send an ACK packet
  // to host once every second to alert it to our presence.
  if((t - lastAckTime) >= 1000) {
    Serial.print("Ada\n"); // Send ACK string to host
    lastAckTime = t; // Reset counter

    // If no data received for an extended time, turn off all LEDs.
    if(SerialTimeout != 0 && (t - lastByteTime) >= (uint32_t) SerialTimeout * 1000) {
      memset(leds, 0, Num_Leds * sizeof(struct CRGB)); //filling Led array by zeroes
      FastLED.show();
      mode = Header;
      lastByteTime = t; // Reset counter
    }
  }
}

void serialFlush(){
  while(Serial.available() > 0) {
    Serial.read();
  }
}
