#include <FastLED.h>

// The LuMini rings need two data pins connected
#define DATA_PIN 11
#define CLOCK_PIN 13




// Define the array of leds
#define NUM_LEDS 60 //3 Inch 
CRGB ring[NUM_LEDS];
uint8_t ringMap[255];
uint8_t rotation = 0;

void fadeAll(uint8_t scale = 250)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    ring[i].nscale8(scale);
  }
}
float angleRangePerLED = 256.0 / (NUM_LEDS); //A single LED will take up a space this many degrees wide.

// see schematic for pin assignments
#define heaterPin 21 
#define sck2 1
#define cs2 3
#define mosi2 5
#define blank 9
#define ign_input 7

int rpm = 8000;
bool NRup = true;
unsigned long tau = 0;
// Create an IntervalTimer object 
IntervalTimer myTimer;
bool displayFlag = false;
bool blinkFlag = false;
bool blinkEven = false;
unsigned long blinkCount = 0;
#define blinkDelay 75
int vfdCount = 0;

// masks for vfd grid
#define threeMask 0b0000000000100000
#define twoMask 0b0000000000000100
#define oneMask 0b0000100000000000
#define zeroMask 0b0000000010000000


void setup()
{
  //Configure pin outputs
  pinMode(sck2, OUTPUT);
  pinMode(cs2, OUTPUT);
  pinMode(mosi2, OUTPUT);
  pinMode(blank, OUTPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(7, INPUT_PULLUP);
  digitalWrite(blank, LOW); // enable VFD
  //analogWriteFrequency(heaterPin, 600);
  //analogWriteResolution(8);
  analogWrite(heaterPin, 150);// turn on heater

  //FastLED setup
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(ring, NUM_LEDS);
  FastLED.setBrightness(32);
  populateMap();

  myTimer.begin(frameISR, 1500);


  //RPM counter interrupt
  attachInterrupt(digitalPinToInterrupt(ign_input), rpm_ISR, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:

rpm = 1000000 / tau * 60 / 2; //convert microseconds to rps to rpm (2 ignition events per revolution) 
if(displayFlag)
{
    //Display LED
  displayLED(rpm);
  //Display VFD
  displayVFD(vfdCount, rpm);
  displayFlag = false;
} 
delayMicroseconds(1000);

}

void populateMap () //we map LED's to a 360 degree circle where 360 == 255
{
  for (int ledNum = 0; ledNum < NUM_LEDS - 10; ledNum++) //Loops through each LED and assigns it to it's range of angles
  {
    for (int j = round(ledNum * angleRangePerLED * 6/5); j < round((ledNum + 1) * angleRangePerLED*6/5); j++)
    {

      if (ledNum <= 25)
      {
        ringMap[j] = 25 - ledNum;
      }
      else if (ledNum > 25)
      {
        ringMap[j] = 85 - ledNum;
      }

      
    }
  }
  return;
}

void frameISR()
{
  //set display flag
  displayFlag = true;
}


//This function displays one digit at a time
//it has to parse rpm into the relevant digit
void displayVFD(int count, int rpm)
{
  static int dec = 0;//decimal
  static int seg = 0;//seven seg
  static int out = 0;//masked output
  static const uint16_t dec2seg[10] = 
  {
    0b0000011100001011,//0
    0b0000010000000010,//1
    0b0000010100011001,//2
    0b0000010100010011,//3
    0b0000011000010010,//4
    0b0000001100010011,//5
    0b0000001100011011,//6
    0b0000010100000010,//7
    0b0000011100011011,//8
    0b0000011100010011//9
  };

  if(count < 1)// if it's zero, or the least significant
    {
      dec = rpm % 10; // parse out 1's column
      seg = dec2seg[dec];// convert to binary in seven seg
      out = seg | zeroMask;
      vfdDigit(out);
      vfdCount = 1;
      return;
    }
  else if (count == 1) //grab 10's
    {
      dec = rpm % 100;
      dec = dec / 10;
      seg = dec2seg[dec];
      out = seg | oneMask;
      vfdDigit(out);
      vfdCount = 2;
      return;
    }
  else if (count == 2) //grab 100's
    {
      dec = rpm % 1000;
      dec = dec / 100;
      seg = dec2seg[dec];
      out = seg | twoMask;
      vfdDigit(out);
      vfdCount = 3;
      return;
    }
  else if (count > 2) // grab 1000's
    {
      dec = rpm / 1000;
      seg = dec2seg[dec];
      out = seg | threeMask;
      vfdDigit(out);
      vfdCount = 0;
      return;
    }
  else{return;}
}

void displayLED(int rotation){
  if(rpm > 9000 && blinkFlag == false && (millis() - blinkCount > blinkDelay)) //when to turn on
  {
    blinkCount = millis();
    blinkFlag = true;
    brandonValley();
    return;
  }
  else if(rpm > 9000 && blinkFlag == true)
  {
    if ((millis() - blinkCount) < blinkDelay)
    {
      return;
    }
    else //reset
    {
      blinkFlag = false;
      blinkCount = millis();
      blinkEven = !blinkEven;
    }
  }
  rotation = rotation * 255 / 9999 ;
  ring[25] = CHSV(25,255,255);
  ring[20] = CHSV(25,255,255);
  ring[15] = CHSV(25,255,255);
  ring[10] = CHSV(25,255,255);
  ring[5] = CHSV(25,255,255);
  ring[0] = CHSV(45,255,255);
  ring[55] = CHSV(35,255,255);
  ring[50] = CHSV(30,255,255);
  ring[45] = CHSV(20,255,255);
  ring[40] = CHSV(5,255,255);
  ring[35] = CHSV(0,255,255);
  ring[ringMap[rotation]] = CHSV(128, 200, 255);
  FastLED.show();
  fadeAll(250);
  return;
}

void vfdDigit(int letter)
{
  int msByte = letter >> 8;
  int lsByte = letter & 0x00FF;
  
  digitalWrite(cs2, LOW);
  shiftOut(mosi2, sck2, MSBFIRST, msByte);
  shiftOut(mosi2, sck2, MSBFIRST, lsByte);
  digitalWrite(cs2, HIGH);
}

void brandonValley()
{
  if(blinkEven)
  {
    for(int i = 0; i < NUM_LEDS; i+=2)
    {
      ring[i] = CHSV(5,255,255);
    }
  }
  else
  {
    for(int i = 1; i < NUM_LEDS; i+=2)
    {
      ring[i] = CHSV(5,255,255);
    }
  }
}
void rpm_ISR()
{
  static unsigned long lastMicros = 0;
  unsigned long nowMicros = 0;
  nowMicros = micros();
  tau = nowMicros - lastMicros;
  lastMicros = nowMicros;
  return;
}
