# draadloze communicatie proof of concept
minimale hard- en software waarmee aangetoond wordt dat duplex gecommuniceerd kan worden tussen de microcontroller en een smartphone, gebruik makend van **Serial Bluetooth Terminal** (gratis Android-app, Kai Morich)

## bestand

[`wireless communication.ino`](./wireless%20communication.ino)

## aansluiting

| pin | naar |
|-----|------|
| D11 (RX) | HC-06 TXD |
| D12 (TX) | HC-06 RXD (via spanningsdeler 1k/2k, zie /technische tekeningen) |

### configuratie

* HC-06 staat standaard op 9600 baud, 8N1 — zo gebruikt in de code hieronder.
* pairing-code is doorgaans `1234` of `0000`, afhankelijk van je exemplaar — probeer beide als het paren niet meteen lukt.
* in de telefoon: koppel de HC-06 eerst via de normale Bluetooth-instellingen van het toestel, pas daarna opent de terminal-app er een seriële verbinding mee.

### opmerkingen

* de HC-06 werkt op 3,3V-logica: TXD mag rechtstreeks naar de Nano (die leest 3,3V nog als HIGH), maar RXD heeft een spanningsdeler nodig vanaf de TX-pin van de Nano (5V), anders riskeer je de module te beschadigen.
* bewust niet op D0/D1 (hardware-serial) aangesloten, zodat die vrij blijven voor USB-programmeren/debuggen.
* `SoftwareSerial` is niet interrupt-safe tijdens het versturen — voor deze proof of concept geen probleem, maar iets om in het achterhoofd te houden als er later meer tegelijk moet gebeuren.

### gebruiksaanwijzing

1. upload `wireless communication.ino` naar de Nano.
2. koppel de HC-06 met je telefoon (Bluetooth-instellingen, pairing-code zie boven).
3. open **Serial Bluetooth Terminal**, verbind met de gekoppelde HC-06.
4. typ iets in de seriële monitor (Arduino IDE, 9600 baud) → verschijnt op je telefoon.
5. typ iets terug in de app op je telefoon → verschijnt in de seriële monitor.

Werkt dit in beide richtingen, dan is de duplex-communicatie aangetoond.
