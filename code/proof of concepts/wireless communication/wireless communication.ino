/*
  Proof of concept: draadloze (bluetooth) communicatie
  ------------------------------------------------------
  Toont aan dat er duplex gecommuniceerd kan worden tussen de microcontroller
  en een smartphone via de HC-06 bluetooth-module: alles wat je intypt in de
  seriële monitor (USB) wordt doorgestuurd naar de telefoon, en alles wat de
  telefoon stuurt verschijnt in de seriële monitor -- dus in beide richtingen
  tegelijk (duplex), zonder dat één richting op de andere moet wachten.

  Aansluiting:
    D11 (RX) <- HC-06 TXD
    D12 (TX) -> HC-06 RXD (via spanningsdeler 1k/2k, zie /technische tekeningen)
*/

#include <SoftwareSerial.h>

const uint8_t PIN_BT_RX = 11;
const uint8_t PIN_BT_TX = 12;
SoftwareSerial btSerial(PIN_BT_RX, PIN_BT_TX);

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600); // standaard baudrate van een ongewijzigde HC-06
  Serial.println(F("wireless communication proof of concept klaar."));
}

void loop() {
  // USB -> bluetooth
  if (Serial.available()) {
    btSerial.write(Serial.read());
  }
  // bluetooth -> USB
  if (btSerial.available()) {
    Serial.write(btSerial.read());
  }
}
