#include <Bounce2.h>

// Déclaration des 2 boutons
const int pinBtn1 = 0;
const int pinBtn2 = 1;
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();

void setup() {
  Serial.begin(9600);
  
  // Configuration bouton 1
  pinMode(pinBtn1, INPUT);
  debouncer1.attach(pinBtn1);
  debouncer1.interval(25);

  // Configuration bouton 2
  pinMode(pinBtn2, INPUT);
  debouncer2.attach(pinBtn2);
  debouncer2.interval(25);
}

void loop() {
  // Mise à jour des deux boutons
  debouncer1.update();
  debouncer2.update();

  // Gestion bouton 1
  if (debouncer1.rose()) {
    Serial.println("Bouton 1 pressé");
  }

  // Gestion bouton 2
  if (debouncer2.rose()) {
    Serial.println("Bouton 2 pressé");
  }
}
