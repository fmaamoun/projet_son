#include "MyDsp.h"

#define AUDIO_OUTPUTS 1
#define MULT_16 32767

// Durée du fade out (en millisecondes)
#define RELEASE_TIME_MS 150 

MyDsp::MyDsp()
  : AudioStream(AUDIO_OUTPUTS, new audio_block_t*[AUDIO_OUTPUTS]),
    sine(AUDIO_SAMPLE_RATE_EXACT),
    echo(AUDIO_SAMPLE_RATE_EXACT, 10000)
{
  // Réglage de l'écho
  echo.setDel(10000);
  echo.setFeedback(0.1); // feedback élevé -> écho plus prononcé

  // Par défaut, pas de note en cours
  isPlaying = false;
  // Pas de fade en cours
  isReleasing = false;
  // Amplitude initiale = 0
  currentAmplitude = 0.0f;

  // Convertit le temps de release en nombre d'échantillons
  float sr = AUDIO_SAMPLE_RATE_EXACT; // ~44 117 Hz sur Teensy
  releaseFrames = (unsigned int)((RELEASE_TIME_MS / 1000.0f) * sr);
}

MyDsp::~MyDsp()
{
}

// Méthode interne pour changer la fréquence du sine
void MyDsp::setFreq(float freq)
{
  sine.setFrequency(freq);
}

// Lance la note : on remet l'amplitude à 1.0 et on active isPlaying
void MyDsp::noteOn(float freq)
{
  setFreq(freq);
  currentAmplitude = 1.0f;
  isPlaying = true;
  isReleasing = false;
}

// Déclenche le fade out
void MyDsp::noteOff()
{
  isReleasing = true;
}

// Méthode update() - coeur du DSP, appelée en continu
void MyDsp::update(void)
{
  audio_block_t* outBlock[AUDIO_OUTPUTS];

  // Important : on ne fait PLUS de "return" si !isPlaying && !isReleasing,
  // afin de toujours calculer l'écho (queue).
  // => On continue à allouer des blocks même si la note est terminée.

  for (int channel = 0; channel < AUDIO_OUTPUTS; channel++) {
    outBlock[channel] = allocate();
    if (!outBlock[channel]) continue;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      float inOsc = 0.0f;

      // Si on joue ou qu'on est en train de relâcher, on génère l'oscillateur
      if (isPlaying || isReleasing) {
        // On multiplie par currentAmplitude pour faire le fade out
        inOsc = sine.tick() * 0.5f * currentAmplitude;
      }

      // Passe l'oscillateur dans l'écho
      float currentSample = echo.tick(inOsc);

      // Gère la phase de release (fade amplitude de l'oscillateur)
      if (isReleasing) {
        float ampDecrement = 1.0f / (float)releaseFrames;
        currentAmplitude -= ampDecrement;

        if (currentAmplitude <= 0.0f) {
          currentAmplitude = 0.0f;
          isPlaying = false;    // on ne joue plus
          isReleasing = false;  // plus de fade
        }
      }

      // Sécurité anti saturation
      currentSample = max(-1.0f, min(1.0f, currentSample));

      // Conversion float -> int16
      outBlock[channel]->data[i] = (int16_t)(currentSample * MULT_16);
    }

    // Envoi du block à la sortie audio
    transmit(outBlock[channel], channel);
    release(outBlock[channel]);
  }
}
