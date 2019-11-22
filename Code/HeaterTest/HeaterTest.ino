int heaterPin = 21;
void setup() {
  // put your setup code here, to run once:
pinMode(heaterPin,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
analogWrite(heaterPin,170);
}

//Heater pins glow at full tilt, needs tuning
