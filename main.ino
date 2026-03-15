#include <ESP32Servo.h>

// Definirea pinilor (Alege pini care suportă ADC și PWM pe ESP32)
const int potPins[4] = {34, 35, 32, 33};   // Pini de intrare analogică
const int servoPins[4] = {13, 12, 14, 27}; // Pini de ieșire PWM

Servo servos[4];

// Variabile pentru filtrul EMA
float smoothedPotValues[4] = {0, 0, 0, 0};
const float alpha = 0.1; // Coeficient de netezire (0.0 la 1.0). Valori mai mici = filtrare mai puternică, dar răspuns mai lent.

// Variabile pentru Histerezis (Deadband)
int lastServoAngles[4] = {0, 0, 0, 0};
const int deadband = 2; // Pragul în grade. Variațiile mai mici de 2 grade sunt ignorate.

void setup() {
  Serial.begin(115200);

  // Configurarea rezoluției ADC la 12 biți (specific ESP32) -> valori între 0 și 4095
  analogReadResolution(12);

  for (int i = 0; i < 4; i++) {
    // Alocarea timerelor hardware pentru servomotoare
    ESP32PWM::allocateTimer(i);
    servos[i].setPeriodHertz(50); // Frecvența standard pentru MG90
    
    // Atașarea servomotoarelor. Parametrii 500 și 2400 reprezintă lățimea impulsului în microsecunde (min/max).
    servos[i].attach(servoPins[i], 500, 2400); 
  }
}

void loop() {
  for (int i = 0; i < 4; i++) {
    // 1. Citirea valorii brute (0 - 4095)
    int rawPotValue = analogRead(potPins[i]);

    // 2. Aplicarea filtrului EMA
    smoothedPotValues[i] = (alpha * rawPotValue) + ((1.0 - alpha) * smoothedPotValues[i]);

    // 3. Maparea valorii filtrate la unghiul servomotorului (0 - 180 grade)
    int targetAngle = map((int)smoothedPotValues[i], 0, 4095, 0, 180);

    // 4. Aplicarea histerezisului pentru a preveni vibrațiile fine la ax (jitter)
    if (abs(targetAngle - lastServoAngles[i]) >= deadband) {
      servos[i].write(targetAngle);
      lastServoAngles[i] = targetAngle;
    }
  }
  
  delay(15); // Timp de stabilizare pentru bucla principală
}
