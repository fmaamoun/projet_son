#include <Bounce2.h>

// Define pins for white and black keys
const int whiteKeys[] = {0, 1, 2, 3, 4, 5, 9}; // Pins for C, D, E, F, G, A, B
const int blackKeys[] = {14, 16, 17, 22, 13};   // Pins for C#, D#, F#, G#, A#

// Define note names
const char* whiteNotes[] = {"C", "D", "E", "F", "G", "A", "B"};
const char* blackNotes[] = {"C#", "D#", "F#", "G#", "A#"};

// Create Bounce objects for all keys
Bounce whiteDebouncers[7];
Bounce blackDebouncers[5];

void setup() {
  Serial.begin(9600);

  // Initialize white keys
  for (int i = 0; i < 7; i++) {
    pinMode(whiteKeys[i], INPUT);
    whiteDebouncers[i].attach(whiteKeys[i]);
    whiteDebouncers[i].interval(25); // Debounce interval (25 ms)
  }

  // Initialize black keys
  for (int i = 0; i < 5; i++) {
    pinMode(blackKeys[i], INPUT);
    blackDebouncers[i].attach(blackKeys[i]);
    blackDebouncers[i].interval(25); // Debounce interval (25 ms)
  }
}

void loop() {
  // Check white keys
  for (int i = 0; i < 7; i++) {
    whiteDebouncers[i].update();
    if (whiteDebouncers[i].rose()) { // If key is pressed
      Serial.print("White key pressed: ");
      Serial.println(whiteNotes[i]);
    }
  }

  // Check black keys
  for (int i = 0; i < 5; i++) {
    blackDebouncers[i].update();
    if (blackDebouncers[i].rose()) { // If key is pressed
      Serial.print("Black key pressed: ");
      Serial.println(blackNotes[i]);
    }
  }
}
