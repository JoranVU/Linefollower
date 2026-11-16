# basis programma

volgende zaken werken samen in dit template project  

* instelbare parameters mbv. SerialCommands 
  * cyclus tijd
  * debug on/off
  * start/stop
* parameters worden opgeslagen in eeprom geheugen
* start/stop kan ook geactiveerd worden mbv. een externe interrupt
* draadloze communicatie

Met behulp van de cyclus kan je bvb. een knipperende led programmeren.

## bestand

[`basis.ino`](./basis.ino)

## vereiste library

"SerialCommands" door Pedro Pereira (ppedro74) — te installeren via Arduino IDE → Tools → Manage Libraries...

## aansluitingen

| pin | naar |
|-----|------|
| D2  | start/stop-drukknop → GND (intern pulled-up, externe interrupt) |
| D11 (RX) | HC-06 TXD |
| D12 (TX) | HC-06 RXD (via spanningsdeler 1k/2k) |
| D13 | ingebouwde LED, knippert op de cyclus |

## seriële commando's

Zelfde commando's werken zowel via USB-serial (9600 baud) als via bluetooth.

| commando | werking |
|---|---|
| `CYCLE <ms>` | cyclustijd instellen (ms) — zonder argument: huidige waarde tonen |
| `DEBUG <0\|1>` | debugprints aan/uit — zonder argument: huidige waarde tonen |
| `START` | start de cyclus |
| `STOP` | stop de cyclus |
| `STATUS` | toont alle huidige instellingen |

Alle instellingen (inclusief start/stop) worden bij elke wijziging opgeslagen in EEPROM en bij het opstarten terug ingelezen.
