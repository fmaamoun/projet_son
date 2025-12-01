#include "MyDsp.h"
#include <math.h>

// Constants
#define AUDIO_OUTPUTS 1
#define MULT_16 32767
#define RELEASE_TIME_MS 150 

MyDsp::MyDsp()
  : AudioStream(AUDIO_OUTPUTS, new audio_block_t*[AUDIO_OUTPUTS]),
    sine(AUDIO_SAMPLE_RATE_EXACT)
{
  // Initial voice state
  isPlaying = false;
  isReleasing = false;
  currentAmplitude = 0.0f;

  // Convert release duration to number of samples
  float sr = AUDIO_SAMPLE_RATE_EXACT;
  releaseFrames = (unsigned int)((RELEASE_TIME_MS / 1000.0f) * sr);

  // Initialize phase and default mode
  phase = 0.0f;
  phaseIncrement = 0.0f;
  mode = MODE_SINE;
}

MyDsp::~MyDsp() {
}

void MyDsp::setFreq(float freq) {
  sine.setFrequency(freq); // used when mode == SINE
  phaseIncrement = freq / AUDIO_SAMPLE_RATE_EXACT; // for non-sine modes
  phase = 0.0f;
}

void MyDsp::noteOn(float freq) {
  setFreq(freq);
  currentAmplitude = 1.0f;
  isPlaying = true;
  isReleasing = false;
}

void MyDsp::noteOff() {
  isReleasing = true;
}

void MyDsp::setMode(int newMode) {
  mode = newMode;
}

void MyDsp::update(void) {
  audio_block_t* outBlock[AUDIO_OUTPUTS];

  // Process each output channel
  for (int channel = 0; channel < AUDIO_OUTPUTS; channel++) {
    outBlock[channel] = allocate();
    if (!outBlock[channel]) continue;

    // Fill the audio block with generated samples
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      float inOsc = 0.0f;
      if (isPlaying || isReleasing) {
        float sampleVal = 0.0f;

        // Waveform generation per current mode
        if (mode == MODE_SINE) {
          sampleVal = sine.tick();
        } else if (mode == MODE_SQUARE) {
          sampleVal = (phase < 0.5f) ? 1.0f : -1.0f;
          phase += phaseIncrement;
          if (phase >= 1.0f) phase -= 1.0f;
        } else if (mode == MODE_SAW) {
          sampleVal = 2.0f * phase - 1.0f;
          phase += phaseIncrement;
          if (phase >= 1.0f) phase -= 1.0f;
        } else if (mode == MODE_TRIANGLE) {
          sampleVal = 4.0f * fabs(phase - 0.5f) - 1.0f;
          phase += phaseIncrement;
          if (phase >= 1.0f) phase -= 1.0f;
        } else if (mode == MODE_PULSE) {
          sampleVal = (phase < 0.25f) ? 1.0f : -1.0f;
          phase += phaseIncrement;
          if (phase >= 1.0f) phase -= 1.0f;
        }
        inOsc = sampleVal * 0.5f * currentAmplitude;
      }

      float currentSample = inOsc;

      // Handle fade-out (decrement amplitude each sample)
      if (isReleasing) {
        float ampDecrement = 1.0f / (float)releaseFrames;
        currentAmplitude -= ampDecrement;
        if (currentAmplitude <= 0.0f) {
          currentAmplitude = 0.0f;
          isPlaying = false;
          isReleasing = false;
        }
      }

      // Clipping to avoid saturation
      currentSample = max(-1.0f, min(1.0f, currentSample));
      outBlock[channel]->data[i] = (int16_t)(currentSample * MULT_16);
    }

    // Transmit the audio block
    transmit(outBlock[channel], channel);

    // Release the block
    release(outBlock[channel]);
  }
}