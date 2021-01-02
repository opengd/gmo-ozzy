#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <AudioDelay.h>

#define CONTROL_RATE 128

Oscil <2048, AUDIO_RATE> aSin1(SIN2048_DATA);
Oscil <2048, CONTROL_RATE> kVib1(SIN2048_DATA);

Oscil <2048, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <2048, CONTROL_RATE> kVib2(SIN2048_DATA);

int trig1State = 0;
int trig2State = 0;

void setup() {
  // put your setup code here, to run once:
  aSin1.setFreq(440);
  kVib1.setFreq(6.5f);
  aSin2.setFreq(440);
  kVib2.setFreq(6.5f);
  startMozzi(CONTROL_RATE);
  pinMode(3, INPUT); // Enable pin 3 as input for reset trigger
  pinMode(2, INPUT); // Enable pin 3 as input for reset trigger
  pinMode(13, OUTPUT); // Enable Pin 13 as Teensy power status LED output (onboard the Teensy)
  digitalWrite(13, HIGH); // Power status LED ON to let us know that the Teensy is recieving proper Vcc.
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void updateControl() {
  trig1State = digitalRead(3); // read reset trigger input
  trig2State = digitalRead(2); // read reset trigger input

  updateVoice(1, 2, 7, 8, &aSin1, &kVib1);
  updateVoice(5, 6, 3, 4, &aSin2, &kVib2);
}

int updateAudio() {
  float oc1 = (trig1State == HIGH) ? aSin1.next() : 0;
  float oc2 = (trig2State == HIGH) ? aSin2.next() : 0;
  return (oc1 + oc2) * 100;
}

void loop() {
  // put your main code here, to run repeatedly:
  audioHook();
}

void updateVoice(int pitchPotPin, int pitchCvPin, int vibratoPotPin, int vibratoCvPin, Oscil<2048, AUDIO_RATE>* pitchOscil, Oscil <2048, CONTROL_RATE>* vibratoOscil)
{
  int pitchMath = getPotAndCv(pitchPotPin, pitchCvPin);
  int vibratoMath = getPotAndCv(vibratoPotPin, vibratoCvPin);

  float vibratoDepth = mapfloat(vibratoMath, 1, 1023, 0.01, 5.0);

  int pitch = map(pitchMath, 1, 1023, 1, 2047);

  float vibrato = vibratoDepth * vibratoOscil->next();

  pitchOscil->setFreq(pitch + vibrato);
}

int getPotAndCv(int potPin, int cvPin)
{
  int potRaw = mozziAnalogRead(potPin); // read pot 1 on Pin (A1) value is 1-1023
  int cvRaw = (mozziAnalogRead(cvPin) - 512); // read pitch CV on Pin (A2) value is 1-1023.

  return (cvRaw > 512) // If the reading is above 512 we know it's a positive value
         ? (potRaw - cvRaw) // So we add it to the Pot
         : (potRaw + cvRaw); // So we subtract it from the Pot
}
