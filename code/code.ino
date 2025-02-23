#include <Audio.h>
#include "MyDsp.h"
#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

// ==================== AUDIO OBJECTS ====================
MyDsp voice1, voice2, voice3, voice4;
AudioMixer4 mixer;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;

// Audio connections
AudioConnection patchCord1(voice1, 0, mixer, 0);
AudioConnection patchCord2(voice2, 0, mixer, 1);
AudioConnection patchCord3(voice3, 0, mixer, 2);
AudioConnection patchCord4(voice4, 0, mixer, 3);
AudioConnection patchCord5(mixer, 0, out, 0);
AudioConnection patchCord6(mixer, 0, out, 1);

// ==================== VOLUME CONTROL ====================
const int POT_PIN = A0;
float volume = 0.5;
ResponsiveAnalogRead analog(POT_PIN, true);

// ==================== KEY CONFIGURATION ====================
#define NUM_KEYS 7
const int whiteKeys[NUM_KEYS] = {0, 1, 2, 3, 4, 5, 9};
const char* whiteNotes[NUM_KEYS] = {"C", "D", "E", "F", "G", "A", "B"};
const float whiteFreqs[NUM_KEYS] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88};

Bounce whiteDebouncers[NUM_KEYS];

// ==================== MODE BUTTON ====================
const int modeButtonPin = 16;
Bounce modeButton;
int currentMode = 0;  // 0-4: SINE, SQUARE, SAW, TRIANGLE, PULSE

// ==================== VOICE MANAGEMENT ====================
#define NUM_VOICES 4
bool voicePlaying[NUM_VOICES] = {false};
float voiceFreq[NUM_VOICES] = {0};

int findFreeVoice() {
  for (int i = 0; i < NUM_VOICES; i++) {
    if (!voicePlaying[i]) return i;
  }
  return -1;
}

int findVoiceWithFreq(float freq) {
  for (int i = 0; i < NUM_VOICES; i++) {
    if (voicePlaying[i] && fabs(voiceFreq[i] - freq) < 0.1) return i;
  }
  return -1;
}

void updateAllVoicesMode() {
  voice1.setMode(currentMode);
  voice2.setMode(currentMode);
  voice3.setMode(currentMode);
  voice4.setMode(currentMode);
}

void setup() {
  AudioMemory(16);
  audioShield.enable();
  audioShield.volume(volume);
  
  Serial.begin(9600);
  
  analog.setActivityThreshold(20);
  analog.setSnapMultiplier(0.03);
  analog.enableEdgeSnap();

  for (int i = 0; i < NUM_KEYS; i++) {
    pinMode(whiteKeys[i], INPUT);
    whiteDebouncers[i].attach(whiteKeys[i]);
    whiteDebouncers[i].interval(25);
  }

  pinMode(modeButtonPin, INPUT_PULLUP);
  modeButton.attach(modeButtonPin);
  modeButton.interval(25);
  
  updateAllVoicesMode();
  Serial.println("Mode: SINE");
}

void loop() {
  analog.update();
  if (analog.hasChanged()) {
    volume = analog.getValue() / 1023.0;
    audioShield.volume(volume);
    Serial.print("Volume: ");
    Serial.println(volume, 2);
  }

  modeButton.update();
  if (modeButton.rose()) {
    currentMode = (currentMode + 1) % 5;
    updateAllVoicesMode();
    Serial.print("Mode changed to: ");
    switch(currentMode) {
      case 0: Serial.println("SINE"); break;
      case 1: Serial.println("SQUARE"); break;
      case 2: Serial.println("SAW"); break;
      case 3: Serial.println("TRIANGLE"); break;
      case 4: Serial.println("PULSE"); break;
    }
  }

  for (int i = 0; i < NUM_KEYS; i++) {
    whiteDebouncers[i].update();

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
        Serial.print("Note ON: ");
        Serial.println(whiteNotes[i]);
      }
    }

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
        voicePlaying[v] = false;
        Serial.print("Note OFF: ");
        Serial.println(whiteNotes[i]);
      }
    }
  }
}
