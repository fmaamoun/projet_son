#include <Audio.h>
#include "MyDsp.h"

MyDsp myDsp;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(myDsp,0,out,0);
AudioConnection patchCord1(myDsp,0,out,1);



#include <Bounce2.h>

// Volume control variables
int volume = 127;            // Default volume (MIDI range 0-127)
int prevVolume = volume;     // Previous volume for change detection

// Define pins for white and black keys
const int whiteKeys[] = {0, 1, 2, 3, 4, 5, 9}; // GPIO pins for C, D, E, F, G, A, B
const int blackKeys[] = {15, 16, 17, 22, 13};  // GPIO pins for C#, D#, F#, G#, A#

// Note labels for serial output
const char* whiteNotes[] = {"C", "D", "E", "F", "G", "A", "B"};
const char* blackNotes[] = {"C#", "D#", "F#", "G#", "A#"};

const float whiteFreqs[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88};
const float blackFreqs[] = {277.18, 311.13, 369.99, 415.30, 466.16};

// Debouncers for key stabilization
Bounce whiteDebouncers[7];
Bounce blackDebouncers[5];

void setup() {
  AudioMemory(2);
  audioShield.enable();
  audioShield.volume(0.5);

  Serial.begin(9600);
  
  // Potentiometer setup (A0 = pin 14 on Teensy)
  pinMode(A0, INPUT);

  // Initialize white keys with debouncing
  for (int i = 0; i < 7; i++) {
    pinMode(whiteKeys[i], INPUT);
    whiteDebouncers[i].attach(whiteKeys[i]);
    whiteDebouncers[i].interval(25); // 25ms debounce time
  }

  // Initialize black keys with debouncing
  for (int i = 0; i < 5; i++) {
    pinMode(blackKeys[i], INPUT);
    blackDebouncers[i].attach(blackKeys[i]);
    blackDebouncers[i].interval(25);
  }
}

void loop() {
  // Read potentiometer value
  int analogValue = analogRead(A0);
  // Convert 0-1023 to 0-100%
  volume = map(analogValue, 0, 1023, 0, 100);
  
  // Detect significant volume changes (threshold prevents jitter)
  if(abs(volume - prevVolume) > 2) {
    Serial.print("Volume: ");
    Serial.print(volume);
    Serial.println("%");
    prevVolume = volume;
  }

  // Check white key status
  for (int i = 0; i < 7; i++) {
    whiteDebouncers[i].update();
    if (whiteDebouncers[i].rose()) { // Key pressed
      Serial.print("White key pressed: ");
      Serial.println(whiteNotes[i]);
      myDsp.setFreq(whiteFreqs[i]);
    }
  }

  // Check black key status
  for (int i = 0; i < 5; i++) {
    blackDebouncers[i].update();
    if (blackDebouncers[i].rose()) { // Key pressed
      Serial.print("Black key pressed: ");
      Serial.println(blackNotes[i]);
      myDsp.setFreq(blackFreqs[i]);
    }
  }
}

