/*
  Proof of concept: start/stop via hardware interrupt
  ----------------------------------------------------
  Toont aan dat een druk op de start/stop-knop correct gedetecteerd wordt via
  een hardware interrupt (niet via polling in de loop), en toont de toestand
  (running/gestopt) op de ingebouwde LED en op de seriële monitor.

  Aansluiting:
    D2  start/stop-drukknop, andere pool naar GND (intern pulled-up op D2)
    D13 ingebouwde LED: AAN = running, UIT = gestopt
*/

const uint8_t PIN_BUTTON = 2;           // externe interrupt (INT0)
const uint8_t PIN_LED    = LED_BUILTIN; // D13

volatile bool toggleRequested = false;
bool running = false;

void isr_button() {
  toggleRequested = true; // ISR zo kort mogelijk houden: enkel een vlag zetten
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), isr_button, FALLING);

  Serial.begin(9600);
  Serial.println(F("interrupt proof of concept klaar. Druk op de knop..."));
}

void loop() {
  static unsigned long lastToggleMs = 0;

  if (toggleRequested) {
    toggleRequested = false;
    unsigned long now = millis();
    if (now - lastToggleMs >= 200) { // ontdendering, 200 ms
      lastToggleMs = now;
      running = !running;
      digitalWrite(PIN_LED, running ? HIGH : LOW);
      Serial.println(running ? F("STATUS=START") : F("STATUS=STOP"));
    }
  }

  // de rest van loop() blijft hier volledig vrij: de interrupt blokkeert niets
}
