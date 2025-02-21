#include <Audio.h>
#include "MyDsp.h"

// ---------- OBJETS AUDIO ------------
MyDsp myDsp;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(myDsp, 0, out, 0);
AudioConnection patchCord1(myDsp, 0, out, 1);

#include <Bounce2.h>

// -------------- VOLUME ---------------
int volume = 127;       // 0..127 (approx MIDI)
int prevVolume = volume;

// ---------- TOUCHES (GPIO) -----------
const int whiteKeys[] = {0, 1, 2, 3, 4, 5, 9}; // C, D, E, F, G, A, B
const int blackKeys[] = {15, 16, 17, 22, 13};  // C#, D#, F#, G#, A#

// Étiquettes de notes (debug)
const char* whiteNotes[] = {"C", "D", "E", "F", "G", "A", "B"};
const char* blackNotes[] = {"C#", "D#", "F#", "G#", "A#"};

// Fréquences associées
const float whiteFreqs[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88};
const float blackFreqs[] = {277.18, 311.13, 369.99, 415.30, 466.16};

// ----- DEBOUNCE (Bounce2) ------------
Bounce whiteDebouncers[7];
Bounce blackDebouncers[5];

void setup() {
  // Initialisation audio
  AudioMemory(8);
  audioShield.enable();
  audioShield.volume(0.5);  // Volume du codec (0.0 -> 1.0)

  // Série pour debug
  Serial.begin(9600);

  // Potentiomètre sur A0
  pinMode(A0, INPUT);

  // Initialisation touches blanches
  for (int i = 0; i < 7; i++) {
    pinMode(whiteKeys[i], INPUT);
    whiteDebouncers[i].attach(whiteKeys[i]);
    whiteDebouncers[i].interval(25); // ms
  }

  // Initialisation touches noires
  for (int i = 0; i < 5; i++) {
    pinMode(blackKeys[i], INPUT);
    blackDebouncers[i].attach(blackKeys[i]);
    blackDebouncers[i].interval(25);
  }
}

void loop() {
  // Lire le potentiomètre (0..1023) -> volume (0..100%)
  int analogValue = analogRead(A0);
  volume = map(analogValue, 0, 1023, 0, 100);

  // Afficher volume si variation > 2%
  if (abs(volume - prevVolume) > 2) {
    Serial.print("Volume: ");
    Serial.print(volume);
    Serial.println("%");
    prevVolume = volume;
  }

  // Mettre à jour les debouncers
  for (int i = 0; i < 7; i++) {
    whiteDebouncers[i].update();

    // 1) Appui (rose) => noteOn
    if (whiteDebouncers[i].rose()) {
      myDsp.noteOn(whiteFreqs[i]);
      Serial.print("White key pressed: ");
      Serial.println(whiteNotes[i]);
    }

    // 2) Relâchement (fell) => noteOff
    if (whiteDebouncers[i].fell()) {
      myDsp.noteOff();
      Serial.print("White key released: ");
      Serial.println(whiteNotes[i]);
    }
  }

  for (int i = 0; i < 5; i++) {
    blackDebouncers[i].update();

    // 1) Appui (rose) => noteOn
    if (blackDebouncers[i].rose()) {
      myDsp.noteOn(blackFreqs[i]);
      Serial.print("Black key pressed: ");
      Serial.println(blackNotes[i]);
    }

    // 2) Relâchement (fell) => noteOff
    if (blackDebouncers[i].fell()) {
      myDsp.noteOff();
      Serial.print("Black key released: ");
      Serial.println(blackNotes[i]);
    }
  }
}
