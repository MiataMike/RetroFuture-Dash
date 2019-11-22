int heaterPin = 21; // see schematic for pin assignments
int sck2 = 1; 
int cs2 = 3;
int mosi2 = 5;
int blank = 9;

void setup() {
  // put your setup code here, to run once:
pinMode(heaterPin, OUTPUT);
pinMode(sck2, OUTPUT);
pinMode(cs2, OUTPUT);
pinMode(mosi2, OUTPUT);
pinMode(blank, OUTPUT);

digitalWrite(blank, LOW); // enable VFD
analogWriteFrequency(heaterPin, 1);
//analogWriteResolution(8);
analogWrite(heaterPin, 100);// turn on heater

}

void loop() {
  // put your main code here, to run repeatedly:
//
  vfdDigit(0b0000010000111011); //D
  //delay(1);
  vfdDigit(0b0000011101011110); //A
  //delay(1);
  vfdDigit(0b0000100000011010); //N
  //delay(100);
  vfdDigit(0b0000011010011010); //K
  //delay(100);
  //digitalWrite(blank, HIGH);
  //delay(500);
  //digitalWrite(blank, LOW);
}

//Heater pins glow at full tilt, needs tuning
void vfdDigit(int letter)
{
  int msByte = letter >> 8;
  int lsByte = letter & 0x00FF;
  
  digitalWrite(cs2, LOW);
  shiftOut(mosi2, sck2, MSBFIRST, msByte);
  shiftOut(mosi2, sck2, MSBFIRST, lsByte);
  digitalWrite(cs2, HIGH);
}
