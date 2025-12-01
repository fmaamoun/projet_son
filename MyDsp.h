#ifndef MYDSP_H
#define MYDSP_H

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"
#include "Sine.h"

// Synthesizer modes
enum SynthMode {
  MODE_SINE,
  MODE_SQUARE,
  MODE_SAW,
  MODE_TRIANGLE,
  MODE_PULSE
};

// Custom DSP class for a simple synthesizer voice
class MyDsp : public AudioStream {
  public:
    MyDsp();
    ~MyDsp();

    // update() method called continuously by the Audio engine
    virtual void update(void);

    // Start the note at the given frequency
    void noteOn(float freq);

    // Trigger a fade-out to stop the note smoothly
    void noteOff();

    // Change the synth mode (waveform)
    void setMode(int newMode);

  private:
    // Set the oscillator frequency and update the phase (used for non-sinusoidal modes)
    void setFreq(float freq);

    // Note state
    bool isPlaying;
    bool isReleasing;
    float currentAmplitude;

    // Number of samples used for the fade-out
    unsigned int releaseFrames;

    // Oscillator used for SINE mode
    Sine sine;
    
    // For non-sinusoidal modes, use a phase accumulator
    float phase;
    float phaseIncrement;
    
    // Current synth mode
    int mode;
};

#endif