#include <IRremote.h>
#include <NewTone.h>

//Defining the frequency codes (in decimal) for the infrared signals 
#define BUTTON_1 4126243066
#define BUTTON_2 377844950
#define BUTTON_3 2305726874
#define BUTTON_4 3089630874
#define BUTTON_5 2320905746
#define BUTTON_6 2332256118
#define BUTTON_7 3219541942
#define BUTTON_PLAY 2162296698
#define BUTTON_UP 310486390
#define BUTTON_DOWN 662553406
#define BUTTON_MUTE 2728010551

//Defning pins
#define TONE_PIN 5
int RECV_PIN = 11;
int SHARP_PIN = 4;
int RECORD_PIN = 8;

//Initializing the scale (from C to B)
int scale[] = { 262, 294, 330, 349, 392, 440, 494, 523 };
int octave = 2;

//Initialize the necessary variables
int note_duration[100] = {0};
int recording[100] = {0};
int count = 0;
int tone_freq;
int start_time;

//Initialize variables for more consistent button presses
volatile long last_debounce_time = 0;
volatile long last_lengthen_time = 0;
long debounce_delay = 200;

//Variables to be changed in interrupts
volatile float sharpenScaler = 1;
volatile int state;
volatile boolean lengthening = false;
volatile boolean record = false;

//Initialize IR Recieving
IRrecv irrecv(RECV_PIN);
decode_results results;

volatile int timer1_overflow_count = 0;

void setup()
{
  //Begin interrupts, set up pins
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  attachInterrupt(0,debounce_lengthen, CHANGE);
  attachInterrupt(1, debounce_record, FALLING);
  pinMode(SHARP_PIN, INPUT);
  pinMode(RECV_PIN, INPUT);
  pinMode(RECORD_PIN, OUTPUT);
  state = digitalRead(SHARP_PIN);
  
  Serial.begin(9600);
  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1=0;
  TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
  TCCR1B =  ( _BV(CS12));
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

void loop() {
  //Writes if it is currently recording
  digitalWrite(RECORD_PIN, record);
  if (!lengthening) {
    //If the button to lengthen the sound is not pressed, it will stop the tone
    noNewTone(TONE_PIN);
  }
  //Makes the current note sharp if the sharp button is pressed down
  if (digitalRead(SHARP_PIN) != state) {
    state = digitalRead(SHARP_PIN);
    sharpen();
  }
  //Read the results from the infrared sensor
  if (irrecv.decode(&results)) {
    if (record && count >= 0) {
      //If its currently recording, add notes to the array, and keep track of the duration in-between
      note_duration[count] = new_millis() - start_time;
      start_time = new_millis();
      recording[count] = read_infrared();
    }
    else {
      read_infrared();
    }
    irrecv.resume(); // Receive the next value
  }
  delay(100);
}

void play() {
  //This function plays the recording when the "INFO" buton is pressed
  count = 0;
  while (recording[count] != 0) {
    //Delay the amount of time in-between note presses, loops until theres no notes
    delay(note_duration[count]);
    //Plays the tone at the given index
    NewTone(TONE_PIN, recording[count], 100);
    //Increment the counter
    count++;
    if (irrecv.decode(&results)) {
      //Checks to see if the mute button has been pressed, if so, stop the playback
      if (results.value == 2728010551) {
        break;
      }
      irrecv.resume();
    }

  }
  count = 0;
}
void sharpen() {
  //Multiplies the note by a scalar, this makes notes sharp
  sharpenScaler = 1.05949 / sharpenScaler;
  Serial.println("SHARPENING");
}
void stopTone() {
  //This function stops the current tone
  noNewTone(TONE_PIN); // Turn off the tone.
}


void debounce_record() {
  //We found buttons to be unreliable, so we check if the button was pressed multiple times
  //In a given interval. If so, we only trigger the function once.
  if ((new_millis() - last_debounce_time) >= debounce_delay) {
    toggle_record();

  }
  //Keeps track of the last time the button was pressed
  last_debounce_time = new_millis();
}

void debounce_lengthen(){
  //Same as the debounce_record function, but to lengthen notes
  if((new_millis()-last_lengthen_time)>=debounce_delay){
    lengthen();
  }
  last_lengthen_time = new_millis();
}

void lengthen() {
  //This function is called from the interrupt function
  lengthening = !lengthening;
}
void toggle_record() {
  //This toggles the recording - stops and starts it
  count = 0;
  //Keep track of the start time, set record on/off
  start_time = new_millis();
  record = !record;
  //If the function is recording, reset the array of notes
  if (record) {
    for (int i = 0 ; i < 50 ; i++) {
      recording[i] = 0;
    }
  }
}
float read_infrared() {
  //This function parses infrared values
  //Serial.println(results.value);
  tone_freq = 0;
  if (results.value == BUTTON_1 ) {
    //For buttons 1-7, it gets the note. Then it adjusts the frequency according to what octave you're on and if it's sharp or natural
    NewTone(TONE_PIN, scale[0]*sharpenScaler * octave);
    tone_freq = scale[0] * sharpenScaler * octave;
    count++;
    //Create the note, set a variable so we can record it
  }
  else if (results.value == BUTTON_2 ) {
    NewTone(TONE_PIN, scale[1]*sharpenScaler * octave);
    tone_freq = scale[1] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == BUTTON_3 ) {
    NewTone(TONE_PIN, scale[2]*sharpenScaler * octave);
    tone_freq = scale[2] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == BUTTON_4 ) {
    NewTone(TONE_PIN, scale[3]*sharpenScaler * octave);
    tone_freq = scale[3] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == BUTTON_5 ) {
    NewTone(TONE_PIN, scale[4]*sharpenScaler * octave);
    tone_freq = scale[4] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == BUTTON_6 ) {
    NewTone(TONE_PIN, scale[5]*sharpenScaler * octave);
    tone_freq = scale[5] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == BUTTON_7 ) {
    NewTone(TONE_PIN, scale[6]*sharpenScaler * octave);
    tone_freq = scale[6] * sharpenScaler * octave;
    count++;
  }
  else if (results.value == BUTTON_UP ) {
    //Frequencies between octaves are a scalar multiple of 2, so if BUTTON_UP is pressed, the octave will be increased
    if (octave <= 16) {
      octave *= 2;
    }
  }
  else if (results.value == BUTTON_DOWN ) {
    if (octave != 1) {
      octave /= 2;
    }
  } 
  else if (results.value == BUTTON_MUTE ) {
    noNewTone(TONE_PIN);
  } 
  else if (results.value == BUTTON_PLAY) {
    play();
  }
  if (count > 49) {
    //automatically stop recording once limit of recording is reached
    toggle_record();
  }
  return tone_freq; //returns the frequency played to record 
}

ISR(TIMER1_OVF_vect)
{
  TCNT1=0;
  timer1_overflow_count++;
}

unsigned long new_millis()
{
  return (TCNT1+timer1_overflow_count*65536)/256.0;
}
