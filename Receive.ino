/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>
#include <NewTone.h>

#define TONE_PIN 5
int RECV_PIN = 11;
int OUT_PIN = 4;
int RECORD_PIN = 12;
int scale[] = { 262, 294, 330, 349, 392, 440, 494, 523 };
int octave = 2;
int note_duration[100] = {0};
int recording[100] = {0};
int count = 0;
int tone_freq;
int start_time;
volatile float sharpenScaler = 1;
volatile int state;
volatile boolean lengthening = false;
volatile boolean record = false;
IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  //attachInterrupt(1, sharpen,CHANGE);
  attachInterrupt(0, lengthen, RISING);
  attachInterrupt(1, toggle_record, RISING);
  pinMode(OUT_PIN, INPUT);
  pinMode(RECV_PIN, INPUT);
  pinMode(RECORD_PIN, OUTPUT);
  state = digitalRead(OUT_PIN);
}

void loop() {
  if (record){
    digitalWrite(RECORD_PIN,HIGH);
  }
  if (!record){
    digitalWrite(RECORD_PIN,LOW);
  }
  if (!lengthening) {
    noNewTone(TONE_PIN);
  }
  if (digitalRead(OUT_PIN) != state) {
    state = digitalRead(OUT_PIN);
    sharpen();
  }
  if (irrecv.decode(&results)) {
    if (record && count>0) {
      note_duration[count-1] = millis() - start_time;
      start_time=millis();
      recording[count-1] = read_infrared();
    }
    else {
      read_infrared();
    }
    irrecv.resume(); // Receive the next value
  }
  if (!record && scale[0]!=0){
    play();
  }
  delay(100);
}

void play(){
  //count = 0;
  while (recording[count]!=0){
    Serial.println(count);
    Serial.println(recording[count]);
    NewTone(TONE_PIN, recording[count],100);
    //delay(100);
    //noNewTone();
    delay(note_duration[count]);
    count++;
  }
}
void sharpen() {
  sharpenScaler = 1.05949 / sharpenScaler;
  Serial.println("SHARPENING");
}
void stopTone() {
  noNewTone(TONE_PIN); // Turn off the tone.
}

void lengthen() {
  lengthening = true;
  Serial.println("SWAG");
  attachInterrupt(0, shorten, FALLING);
}
void shorten() {
  lengthening = false;
  Serial.println("SWAG");
  attachInterrupt(0, lengthen, RISING);
}
void toggle_record() {
  count=0;
  start_time=millis();
  record = !record;

  for (int i = 0 ; i<50; i++){
    Serial.print(recording[i]);
    Serial.print(",");
  }
  Serial.println();
}
float read_infrared() {
  //Serial.println(results.value);
  tone_freq = 0;
  if (results.value == 4126243066) {
    NewTone(TONE_PIN, scale[0]*sharpenScaler * octave);
    tone_freq = scale[0] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == 377844950) {
    NewTone(TONE_PIN, scale[1]*sharpenScaler * octave);
    tone_freq = scale[1] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == 2305726874) {
    NewTone(TONE_PIN, scale[2]*sharpenScaler * octave);
    tone_freq = scale[2] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == 3089630874) {
    NewTone(TONE_PIN, scale[3]*sharpenScaler * octave);
    tone_freq = scale[3] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == 2320905746) {
    NewTone(TONE_PIN, scale[4]*sharpenScaler * octave);
    tone_freq = scale[4] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == 2332256118) {
    NewTone(TONE_PIN, scale[5]*sharpenScaler * octave);
    tone_freq = scale[5] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == 3219541942) {
    NewTone(TONE_PIN, scale[6]*sharpenScaler * octave);
    tone_freq = scale[6] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == 310486390) {
    if (octave <= 16) {
      octave *= 2;
    }
  }
  else if (results.value == 662553406) {
    if (octave != 1) {
      octave /= 2;
    }
  } else if (results.value == 2728010551) {
    noNewTone(TONE_PIN);
  }
  return tone_freq;
}


