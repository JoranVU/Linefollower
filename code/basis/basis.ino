/*
  Linefollower - basisprogramma
  ------------------------------
  Dit is het template-project uit "code/basis": de bouwstenen die klaar moeten
  staan vóór de sensor- en PID-code erbij komt.

    - instelbare parameters (cyclustijd, debug on/off, start/stop) via de
      SerialCommands-library, bereikbaar zowel over USB-serial als draadloos
      via de HC-06 (bluetooth) -- dezelfde commando's werken op beide poorten
    - alle instellingen worden bij elke wijziging weggeschreven naar het
      EEPROM-geheugen, en bij het opstarten terug ingelezen
    - start/stop kan ook omgeschakeld worden met een externe interrupt
      (drukknop op pin 2)
    - de vaste cyclus (in te stellen met CYCLE) wordt hier gedemonstreerd met
      een knipperende LED (D13) -- in "code/finaal" komt hier de PID-regeling
      in de plaats van

  Benodigde library (Arduino IDE -> Tools -> Manage Libraries...):
    "SerialCommands" door Pedro Pereira (ppedro74)

  Aansluitingen (zie /technische tekeningen/elektronisch):
    D2          start/stop-drukknop: andere kant naar GND (intern pulled-up),
                externe interrupt (INT0)
    D11 (RX) <- HC-06 TXD
    D12 (TX) -> HC-06 RXD (via spanningsdeler 1k/2k, zie aansluitschema)
    D13         ingebouwde LED, knippert op de cyclus zolang STATUS=START

  Seriële commando's (zelfde set via USB op 9600 baud, en via bluetooth):
    CYCLE <ms>   cyclustijd instellen in ms (zonder argument: huidige waarde tonen)
    DEBUG <0|1>  debugprints aan/uit (zonder argument: huidige waarde tonen)
    START        start de cyclus
    STOP         stop de cyclus
    STATUS       toont alle huidige instellingen
*/

#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <SerialCommands.h>

// ---------- pinnen ----------
const uint8_t PIN_STARTSTOP = 2;            // externe interrupt (INT0)
const uint8_t PIN_LED       = LED_BUILTIN;  // D13
const uint8_t PIN_BT_RX     = 11;           // <- HC-06 TXD
const uint8_t PIN_BT_TX     = 12;           // -> HC-06 RXD (via spanningsdeler)

// ---------- instellingen, opgeslagen in EEPROM ----------
struct Settings {
  uint8_t magic;
  unsigned long cycleMs;
  bool debug;
  bool running;
};
const uint8_t SETTINGS_MAGIC = 0xA5;
const int EEPROM_ADDR = 0;
Settings settings;

void loadSettings() {
  EEPROM.get(EEPROM_ADDR, settings);
  if (settings.magic != SETTINGS_MAGIC) {
    // eerste keer opstarten (of EEPROM nog leeg) -> standaardwaarden wegschrijven
    settings.magic   = SETTINGS_MAGIC;
    settings.cycleMs = 500;
    settings.debug   = false;
    settings.running = false;
    EEPROM.put(EEPROM_ADDR, settings);
  }
}

void saveSettings() {
  EEPROM.put(EEPROM_ADDR, settings); // schrijft enkel de bytes die echt veranderd zijn
}

// ---------- draadloze communicatie (bluetooth op D11/D12) ----------
SoftwareSerial btSerial(PIN_BT_RX, PIN_BT_TX);

// ---------- SerialCommands: 1 instantie per poort, met dezelfde handlers ----------
// let op: een SerialCommand-object mag maar bij 1 SerialCommands-dispatcher
// horen (zie library-docs) -> per poort een eigen set objecten, die wel
// dezelfde handler-functies hieronder aanroepen
char cmdBufUsb[32];
char cmdBufBt[32];
SerialCommands cmdsUsb(&Serial,   cmdBufUsb, sizeof(cmdBufUsb), "\r\n", " ");
SerialCommands cmdsBt (&btSerial, cmdBufBt,  sizeof(cmdBufBt),  "\r\n", " ");

void cmd_cycle(SerialCommands* sender) {
  char* arg = sender->Next();
  if (arg != NULL) {
    long v = atol(arg);
    if (v > 0) {
      settings.cycleMs = (unsigned long)v;
      saveSettings();
    } else {
      sender->GetSerial()->println(F("ERR: cyclustijd moet > 0"));
    }
  }
  sender->GetSerial()->print(F("CYCLE="));
  sender->GetSerial()->println(settings.cycleMs);
}

void cmd_debug(SerialCommands* sender) {
  char* arg = sender->Next();
  if (arg != NULL) {
    settings.debug = (atoi(arg) != 0);
    saveSettings();
  }
  sender->GetSerial()->print(F("DEBUG="));
  sender->GetSerial()->println(settings.debug ? 1 : 0);
}

void cmd_start(SerialCommands* sender) {
  settings.running = true;
  saveSettings();
  sender->GetSerial()->println(F("STATUS=START"));
}

void cmd_stop(SerialCommands* sender) {
  settings.running = false;
  digitalWrite(PIN_LED, LOW);
  saveSettings();
  sender->GetSerial()->println(F("STATUS=STOP"));
}

void cmd_status(SerialCommands* sender) {
  Stream* s = sender->GetSerial();
  s->print(F("CYCLE="));  s->println(settings.cycleMs);
  s->print(F("DEBUG="));  s->println(settings.debug ? 1 : 0);
  s->print(F("STATUS=")); s->println(settings.running ? F("START") : F("STOP"));
}

void cmd_unrecognized(SerialCommands* sender, const char* cmd) {
  sender->GetSerial()->print(F("ERR: onbekend commando ["));
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println(F("]"));
}

SerialCommand scUsbCycle ("CYCLE",  cmd_cycle);
SerialCommand scUsbDebug ("DEBUG",  cmd_debug);
SerialCommand scUsbStart ("START",  cmd_start);
SerialCommand scUsbStop  ("STOP",   cmd_stop);
SerialCommand scUsbStatus("STATUS", cmd_status);

SerialCommand scBtCycle ("CYCLE",  cmd_cycle);
SerialCommand scBtDebug ("DEBUG",  cmd_debug);
SerialCommand scBtStart ("START",  cmd_start);
SerialCommand scBtStop  ("STOP",   cmd_stop);
SerialCommand scBtStatus("STATUS", cmd_status);

// ---------- start/stop via externe interrupt ----------
volatile bool toggleRequested = false;

void isr_startstop() {
  toggleRequested = true; // ISR zo kort mogelijk houden: enkel een vlag zetten
}

void handleToggleFlag() {
  static unsigned long lastToggleMs = 0;
  if (!toggleRequested) return;
  toggleRequested = false;

  unsigned long now = millis();
  if (now - lastToggleMs < 200) return; // ontdenderen (debounce), 200 ms
  lastToggleMs = now;

  settings.running = !settings.running;
  if (!settings.running) digitalWrite(PIN_LED, LOW);
  saveSettings();

  if (settings.debug) {
    Serial.print(F("[interrupt] STATUS="));
    Serial.println(settings.running ? F("START") : F("STOP"));
  }
}

// ---------- vaste cyclus ----------
unsigned long lastCycleMs = 0;
bool ledState = false;

void cycleTick() {
  if (settings.running) {
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState ? HIGH : LOW);
  }
  if (settings.debug) {
    Serial.print(F("[cycle] t="));
    Serial.print(millis());
    Serial.print(F(" running="));
    Serial.print(settings.running);
    Serial.print(F(" led="));
    Serial.println(ledState);
  }
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_STARTSTOP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_STARTSTOP), isr_startstop, FALLING);

  Serial.begin(9600);
  btSerial.begin(9600); // standaard baudrate van een ongewijzigde HC-06

  loadSettings();

  cmdsUsb.SetDefaultHandler(cmd_unrecognized);
  cmdsUsb.AddCommand(&scUsbCycle);
  cmdsUsb.AddCommand(&scUsbDebug);
  cmdsUsb.AddCommand(&scUsbStart);
  cmdsUsb.AddCommand(&scUsbStop);
  cmdsUsb.AddCommand(&scUsbStatus);

  cmdsBt.SetDefaultHandler(cmd_unrecognized);
  cmdsBt.AddCommand(&scBtCycle);
  cmdsBt.AddCommand(&scBtDebug);
  cmdsBt.AddCommand(&scBtStart);
  cmdsBt.AddCommand(&scBtStop);
  cmdsBt.AddCommand(&scBtStatus);

  Serial.println(F("linefollower basisprogramma klaar."));
  Serial.print(F("CYCLE="));  Serial.println(settings.cycleMs);
  Serial.print(F("DEBUG="));  Serial.println(settings.debug ? 1 : 0);
  Serial.print(F("STATUS=")); Serial.println(settings.running ? F("START") : F("STOP"));
}

void loop() {
  cmdsUsb.ReadSerial();
  cmdsBt.ReadSerial();

  handleToggleFlag();

  unsigned long now = millis();
  if (now - lastCycleMs >= settings.cycleMs) {
    lastCycleMs = now;
    cycleTick();
  }
}
