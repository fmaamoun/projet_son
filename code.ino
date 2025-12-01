#include <Audio.h>
#include "MyDsp.h"
#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>


// ==================== AUDIO OBJECTS ====================
MyDsp voice1, voice2, voice3, voice4;
AudioMixer4 mixer;
AudioMixer4 dryWetMixer;
AudioEffectFreeverbStereo reverb;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;


// Audio connections
AudioConnection patchCord1(voice1, 0, mixer, 0);
AudioConnection patchCord2(voice2, 0, mixer, 1);
AudioConnection patchCord3(voice3, 0, mixer, 2);
AudioConnection patchCord4(voice4, 0, mixer, 3);
AudioConnection patchCord5(mixer, 0, dryWetMixer, 0);  
AudioConnection patchCord6(mixer, 0, reverb, 0);        
AudioConnection patchCord7(reverb, 0, dryWetMixer, 1);  
AudioConnection patchCord8(dryWetMixer, 0, out, 0);     
AudioConnection patchCord9(dryWetMixer, 0, out, 1);     


// ==================== VOLUME CONTROL ====================
const int POT_VOL_PIN = A0;
float volume = 0.5;
ResponsiveAnalogRead analog_vol(POT_VOL_PIN, true);


// ==================== REVERB : ROOM CONTROL ====================
const int POT_ROOM_PIN = A8;
float roomsize = 0.4;
ResponsiveAnalogRead analog_room(POT_ROOM_PIN, true);


// ==================== KEY CONFIGURATION ====================
#define NUM_KEYS 7
const int whiteKeys[NUM_KEYS] = {0, 1, 2, 3, 4, 5, 9};
const char* whiteNotes[NUM_KEYS] = {"C", "D", "E", "F", "G", "A", "B"};
const float whiteFreqs[NUM_KEYS] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88};
Bounce whiteDebouncers[NUM_KEYS];


// ==================== MODE BUTTON ====================
const int modeButtonPin = 17;
Bounce modeButton;
int currentMode = 0;  // 0-4: SINE, SQUARE, SAW, TRIANGLE, PULSE


// ==================== VOICE MANAGEMENT ====================
// Polyphonic voice allocation & tracking (voicePlaying[], voiceFreq[])
#define NUM_VOICES 4
bool voicePlaying[NUM_VOICES] = {false};
float voiceFreq[NUM_VOICES] = {0};

// Return index of a free voice or -1 if none available (used on key press)
int findFreeVoice() {
  for (int i = 0; i < NUM_VOICES; i++) {
    if (!voicePlaying[i]) return i;
  }
  return -1;
}

// Return index of playing voice matching freq (±0.1 Hz), or -1 (used on key release)
int findVoiceWithFreq(float freq) {
  for (int i = 0; i < NUM_VOICES; i++) {
    if (voicePlaying[i] && fabs(voiceFreq[i] - freq) < 0.1) return i;
  }
  return -1;
}

// Set current synth mode on all voice instances (SINE/SQUARE/SAW/...)
void updateAllVoicesMode() {
  voice1.setMode(currentMode);
  voice2.setMode(currentMode);
  voice3.setMode(currentMode);
  voice4.setMode(currentMode);
}

// Distribute mixer gain across active voices so overall level stays consistent
void updateMixerGain() {
    int activeVoices = 0;
    for (int i = 0; i < NUM_VOICES; i++) {
        if (voicePlaying[i]) activeVoices++;
    }
    
    float gain = (activeVoices > 0) ? (1.0 / activeVoices) : 1.0;
    for (int i = 0; i < NUM_VOICES; i++) {
        mixer.gain(i, gain);
    }
}


// ==================== SETUP ====================
void setup() {
  // Audio: reserve buffers and enable audio codec
  AudioMemory(16);
  audioShield.enable();
  audioShield.volume(volume);

  // Serial: enable debugging output
  Serial.begin(9600);

  // Volume pot: configure responsive reader
  analog_vol.setActivityThreshold(20);
  analog_vol.setSnapMultiplier(0.03);
  analog_vol.enableEdgeSnap();

  // Reverb pot: configure responsive reader
  analog_room.setActivityThreshold(40);
  analog_room.setSnapMultiplier(0.03);
  analog_room.enableEdgeSnap();

  // Keys: configure input pins and debouncers
  for (int i = 0; i < NUM_KEYS; i++) {
    pinMode(whiteKeys[i], INPUT);
    whiteDebouncers[i].attach(whiteKeys[i]);
    whiteDebouncers[i].interval(25);
  }

  // Mode button: pin and debouncer
  pinMode(modeButtonPin, INPUT);
  modeButton.attach(modeButtonPin);
  modeButton.interval(25);

  // Voices: apply selected waveform mode to each voice
  updateAllVoicesMode();
  Serial.println("Mode: SINE");

  // Reverb: initial parameters
  reverb.roomsize(roomsize); 
  reverb.damping(0.6);
}


// ==================== LOOP ====================
void loop() {
  // Volume: read pot and apply to output volume
  analog_vol.update();
  if (analog_vol.hasChanged()) {
    volume = analog_vol.getValue() / 1023.0;
    audioShield.volume(volume);
    Serial.print("Volume: ");
    Serial.println(volume, 2);
  }

  // Reverb: read pot and map to room size
  analog_room.update();
  if (analog_room.hasChanged()) {
    int rawRoomsize = analog_room.getValue();
    roomsize = map(rawRoomsize, 915, 1023, 0, 100) / 100.0;
    if(roomsize<0){
      roomsize = 0.0;
    }
    reverb.roomsize(roomsize);
    Serial.print("Reverb: ");
    Serial.println(roomsize, 2);
  }

  // Mode button: cycle waveform mode on press
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
    // Per-key: update debouncer and handle events
    whiteDebouncers[i].update();

    if (whiteDebouncers[i].rose()) {
      // Key press: allocate a free voice and start the note
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
        updateMixerGain();

      }
    }

    if (whiteDebouncers[i].fell()) {
      // Key release: find the voice playing this frequency and stop it
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
        updateMixerGain();

      }
    }
  }
}