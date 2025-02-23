#include "MyDsp.h"

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

  // Initialisation : aucune note en cours
  isPlaying = false;
  isReleasing = false;
  currentAmplitude = 0.0f;

  // Conversion de la durée de release en nombre d'échantillons
  float sr = AUDIO_SAMPLE_RATE_EXACT;
  releaseFrames = (unsigned int)((RELEASE_TIME_MS / 1000.0f) * sr);
}

MyDsp::~MyDsp() {
  // Destructeur (rien de spécifique à libérer)
}

void MyDsp::setFreq(float freq) {
  sine.setFrequency(freq);
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

void MyDsp::update(void) {
  audio_block_t* outBlock[AUDIO_OUTPUTS];

  for (int channel = 0; channel < AUDIO_OUTPUTS; channel++) {
    outBlock[channel] = allocate();
    if (!outBlock[channel]) continue;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      float inOsc = 0.0f;
      // Génère le signal de l'oscillateur si la note est active ou en fade-out
      if (isPlaying || isReleasing) {
        inOsc = sine.tick() * 0.5f * currentAmplitude;
      }

      // Applique l'effet d'écho
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

      // Limite la valeur pour éviter la saturation
      currentSample = max(-1.0f, min(1.0f, currentSample));
      outBlock[channel]->data[i] = (int16_t)(currentSample * MULT_16);
    }

    transmit(outBlock[channel], channel);
    release(outBlock[channel]);
  }
}
