#ifndef faust_teensy_h_
#define faust_teensy_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"

#include "Sine.h"
#include "Echo.h"

class MyDsp : public AudioStream
{
  public:
    MyDsp();
    ~MyDsp();

    // Méthode update() appelée automatiquement par le moteur Audio
    virtual void update(void);

    // Lance la note à une fréquence donnée
    void noteOn(float freq);

    // Déclenche le "fade out" pour arrêter la note en douceur
    void noteOff();

  private:
    // Méthode interne pour modifier la fréquence de l'oscillateur
    void setFreq(float freq);

    // Indique si une note est en cours de lecture
    bool isPlaying;

    // Indique si on est en phase de "release" (fade out)
    bool isReleasing;

    // Amplitude courante (de 1.0 jusqu'à 0.0 quand on relâche)
    float currentAmplitude;

    // Nombre d'échantillons sur lesquels on fait le fade out
    unsigned int releaseFrames;

    // Générateurs (un oscillateur) + effet (Echo)
    Sine sine;
    Echo echo;
};

#endif
