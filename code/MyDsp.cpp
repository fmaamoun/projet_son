#include "MyDsp.h"
#include <math.h> // pour fabs()

#define AUDIO_OUTPUTS 1
#define MULT_16 32767

// Durée du fade-out en millisecondes
#define RELEASE_TIME_MS 150 

MyDsp::MyDsp()
  : AudioStream(AUDIO_OUTPUTS, new audio_block_t*[AUDIO_OUTPUTS]),
    sine(AUDIO_SAMPLE_RATE_EXACT),
    echo(AUDIO_SAMPLE_RATE_EXACT, 10000)
{
  // Réglages de l'effet d'écho
  echo.setDel(10000);
  echo.setFeedback(0.1);

  isPlaying = false;
  isReleasing = false;
  currentAmplitude = 0.0f;

  // Conversion de la durée de release en nombre d'échantillons
  float sr = AUDIO_SAMPLE_RATE_EXACT;
  releaseFrames = (unsigned int)((RELEASE_TIME_MS / 1000.0f) * sr);

  // Initialisation de la phase et du mode
  phase = 0.0f;
  phaseIncrement = 0.0f;
  mode = MODE_SINE; // mode par défaut
}

MyDsp::~MyDsp() {
  // Rien de spécifique à libérer
}

void MyDsp::setFreq(float freq) {
  // Pour le mode SINE, on utilise l'objet sine
  sine.setFrequency(freq);
  // Pour les autres modes, calcule du phaseIncrement (phase normalisée entre 0 et 1)
  phaseIncrement = freq / AUDIO_SAMPLE_RATE_EXACT;
  phase = 0.0f; // réinitialisation de la phase à chaque note
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

void MyDsp::setMode(SynthMode newMode) {
  mode = newMode;
}

void MyDsp::update(void) {
  audio_block_t* outBlock[AUDIO_OUTPUTS];

  for (int channel = 0; channel < AUDIO_OUTPUTS; channel++) {
    outBlock[channel] = allocate();
    if (!outBlock[channel]) continue;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      float inOsc = 0.0f;
      if (isPlaying || isReleasing) {
        float sampleVal = 0.0f;
        // Génération de l'onde selon le mode
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
          // Onde triangulaire : formule classique
          sampleVal = 4.0f * fabs(phase - 0.5f) - 1.0f;
          phase += phaseIncrement;
          if (phase >= 1.0f) phase -= 1.0f;
        } else if (mode == MODE_PULSE) {
          // Onde impulsion : rapport cyclique fixe (ici 25%)
          sampleVal = (phase < 0.25f) ? 1.0f : -1.0f;
          phase += phaseIncrement;
          if (phase >= 1.0f) phase -= 1.0f;
        }
        inOsc = sampleVal * 0.5f * currentAmplitude;
      }

      // Passage par l'effet d'écho
      float currentSample = echo.tick(inOsc);

      // Gestion du fade-out
      if (isReleasing) {
        float ampDecrement = 1.0f / (float)releaseFrames;
        currentAmplitude -= ampDecrement;
        if (currentAmplitude <= 0.0f) {
          currentAmplitude = 0.0f;
          isPlaying = false;
          isReleasing = false;
        }
      }

      // Limitation pour éviter la saturation
      currentSample = max(-1.0f, min(1.0f, currentSample));
      outBlock[channel]->data[i] = (int16_t)(currentSample * MULT_16);
    }

    transmit(outBlock[channel], channel);
    release(outBlock[channel]);
  }
}
