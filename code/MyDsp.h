#ifndef MYDSP_H
#define MYDSP_H

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"
#include "Sine.h"

// Définition des modes de synthèse
enum SynthMode {
  MODE_SINE,
  MODE_SQUARE,
  MODE_SAW,
  MODE_TRIANGLE,
  MODE_PULSE
};

class MyDsp : public AudioStream {
  public:
    MyDsp();
    ~MyDsp();

    // Méthode update() appelée en continu par le moteur Audio
    virtual void update(void);

    // Lance la note à la fréquence indiquée
    void noteOn(float freq);

    // Déclenche le fade-out pour arrêter la note en douceur
    void noteOff();

    // Permet de changer le mode de synthèse (onde)
    void setMode(int newMode);

  private:
    // Modifie la fréquence de l'oscillateur et met à jour la phase (pour modes non-sinusoïdaux)
    void setFreq(float freq);

    // Etat de la note
    bool isPlaying;
    bool isReleasing;
    float currentAmplitude;
    // Nombre d'échantillons pour le fade-out
    unsigned int releaseFrames;

    // Oscillateur utilisé pour le mode SINE
    Sine sine;
    // Pour les modes non-sinusoïdaux, on utilise un accumulateur de phase
    float phase;
    float phaseIncrement;
    
    // Mode de synthèse courant
    int mode;
};

#endif // MYDSP_H