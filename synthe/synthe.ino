#include <Audio.h>
#include "MyDsp.h"
#include <Bounce2.h>

// ==================== OBJETS AUDIO ====================

// Déclaration de 4 voix polyphoniques
MyDsp voice1;
MyDsp voice2;
MyDsp voice3;
MyDsp voice4;

// Mixer 4 voies
AudioMixer4 mixer1;

// Sortie audio I2S
AudioOutputI2S out;

// Connections des voix au mixer
AudioConnection patchCord1(voice1, 0, mixer1, 0);
AudioConnection patchCord2(voice2, 0, mixer1, 1);
AudioConnection patchCord3(voice3, 0, mixer1, 2);
AudioConnection patchCord4(voice4, 0, mixer1, 3);

// Connection du mixer à la sortie stéréo
AudioConnection patchCordMtoOut1(mixer1, 0, out, 0);
AudioConnection patchCordMtoOut2(mixer1, 0, out, 1);

// Contrôle du codec audio
AudioControlSGTL5000 audioShield;

// ==================== VARIABLES DE VOLUME ====================
int volume = 127;       // 0..127 (approx. MIDI)
int prevVolume = volume;

// ==================== TOUCHES (GPIO) ====================

const int whiteKeys[] = {0, 1, 2, 3, 4, 5, 9}; // touches blanches : C, D, E, F, G, A, B

// Étiquettes de notes pour le debug
const char* whiteNotes[] = {"C", "D", "E", "F", "G", "A", "B"};

// Fréquences associées aux notes
const float whiteFreqs[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88};

// ==================== DEBOUNCE (Bounce2) ====================
Bounce whiteDebouncers[7];
Bounce blackDebouncers[5];

// ==================== GESTION DES VOIX ====================
#define NUM_VOICES 4
bool voicePlaying[NUM_VOICES] = {false, false, false, false};
float voiceFreq[NUM_VOICES] = {0, 0, 0, 0};

// Cherche une voix libre parmi les NUM_VOICES
int findFreeVoice() {
  for (int i = 0; i < NUM_VOICES; i++) {
    if (!voicePlaying[i]) {
      return i;
    }
  }
  return -1; // Aucune voix libre
}

// Cherche la voix qui joue la fréquence donnée (tolérance sur les flottants)
int findVoiceWithFreq(float freq) {
  for (int i = 0; i < NUM_VOICES; i++) {
    if (voicePlaying[i] && fabs(voiceFreq[i] - freq) < 0.1) {
      return i;
    }
  }
  return -1;
}

void setup() {
  // Initialisation du système audio
  AudioMemory(12); // allouer plus de mémoire pour 4 voix
  audioShield.enable();
  audioShield.volume(0.5);  // Volume du codec (0.0 -> 1.0)

  // Début du debug sur le port série
  Serial.begin(9600);

  // Potentiomètre sur A0 pour le contrôle du volume
  pinMode(A0, INPUT);

  // Initialisation des touches blanches
  for (int i = 0; i < 7; i++) {
    pinMode(whiteKeys[i], INPUT);
    whiteDebouncers[i].attach(whiteKeys[i]);
    whiteDebouncers[i].interval(25); // ms
  }
}

void loop() {
  // Lecture du potentiomètre (0..1023) et mise à jour du volume (0..100%)
  int analogValue = analogRead(A0);
  volume = map(analogValue, 0, 1023, 0, 100);
  if (abs(volume - prevVolume) > 2) {
    Serial.print("Volume: ");
    Serial.print(volume);
    Serial.println("%");
    prevVolume = volume;
  }

  // ----------------- GESTION DES TOUCHES BLANCHES -----------------
  for (int i = 0; i < 7; i++) {
    whiteDebouncers[i].update();

    // Lors de l'appui, on recherche une voix libre et on lance la note
    if (whiteDebouncers[i].rose()) {
      float freq = whiteFreqs[i];
      int v = findFreeVoice();
      if (v >= 0) {
        switch(v) {
          case 0: voice1.noteOn(freq); break;
          case 1: voice2.noteOn(freq); break;
          case 2: voice3.noteOn(freq); break;
          case 3: voice4.noteOn(freq); break;
        }
        voicePlaying[v] = true;
        voiceFreq[v] = freq;
        Serial.print("White key pressed: ");
        Serial.println(whiteNotes[i]);
      } else {
        Serial.println("Aucune voix libre pour la touche blanche !");
      }
    }

    // Au relâchement, on trouve la voix jouant la note et on lance le fade-out
    if (whiteDebouncers[i].fell()) {
      float freq = whiteFreqs[i];
      int v = findVoiceWithFreq(freq);
      if (v >= 0) {
        switch(v) {
          case 0: voice1.noteOff(); break;
          case 1: voice2.noteOff(); break;
          case 2: voice3.noteOff(); break;
          case 3: voice4.noteOff(); break;
        }
        voicePlaying[v] = false; // On libère immédiatement la voix (vous pouvez l'affiner pour attendre la fin du fade)
        Serial.print("White key released: ");
        Serial.println(whiteNotes[i]);
      }
    }
  }
}