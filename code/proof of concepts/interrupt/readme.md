# start/stop interrupt proof of concept
minimale hard- en software die de correcte werking van een start/stop drukknop aantoont, gebruik makend van een hardware interrupt

## bestand

[`interrupt.ino`](./interrupt.ino)

## aansluiting

| pin | naar |
|-----|------|
| D2  | drukknop → GND (intern pulled-up, externe interrupt INT0) |
| D13 | ingebouwde LED: AAN = running, UIT = gestopt |

## aandachtspunten

* de interrupt-routine (ISR) zet enkel een vlag (`toggleRequested`) — alle verdere verwerking (ontdendering, LED, Serial-print) gebeurt in `loop()`. Een ISR moet altijd zo kort mogelijk blijven.
* ontdendering gebeurt tijdsgebaseerd (`millis()`), met een minimum van 200 ms tussen twee geldige toggles, zodat het mechanisch "springen" van de knop niet als meerdere drukken geteld wordt.
* `FALLING` + `INPUT_PULLUP`: de pin is intern hoog, en de knop trekt hem naar GND bij een druk — geen externe pull-up-weerstand nodig.
* deze schakeling en code komen 1-op-1 terug in `code/basis` (daar bovenop met EEPROM-opslag van de running-status).
