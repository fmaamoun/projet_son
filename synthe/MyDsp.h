#ifndef MYDSP_H
#define MYDSP_H

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"

#include "Sine.h"
#include "Echo.h"

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

  private:
    // Change la fréquence de l'oscillateur interne
    void setFreq(float freq);

    // Indique si une note est en cours de lecture
    bool isPlaying;
    // Indique si le fade-out est en cours
    bool isReleasing;
    // Amplitude actuelle (de 1.0 à 0.0 pendant le fade-out)
    float currentAmplitude;
    // Nombre d'échantillons pour effectuer le fade-out
    unsigned int releaseFrames;

    // Oscillateur et effet d'écho
    Sine sine;
    Echo echo;
};

#endif // MYDSP_H
