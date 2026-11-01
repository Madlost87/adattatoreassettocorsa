## 1. Setup progetto

- [ ] 1.1 Creare progetto PlatformIO `firmware/` per `esp32-s3-devkitc-1`, framework Arduino (core 3.x)
- [ ] 1.2 Impostare build flags `ARDUINO_USB_MODE=0`, `ARDUINO_USB_CDC_ON_BOOT=1`, `ENABLE_CLUTCH=0`
- [ ] 1.3 Creare `firmware/include/pins.h` con il pinout definito in design (D3)
- [ ] 1.4 Aggiungere README con istruzioni di build/flash (porta "USB" della devkit)

## 2. Hardware e moduli pedale

- [ ] 2.1 Disegnare lo schema elettrico della centralina (ESP32-S3, 3 porte GX12, filtri RC 1 kΩ/100 nF, pulsanti) in `hardware/`
- [ ] 2.2 Documentare il pinout del connettore modulo (3V3, SIG, GND, DETECT) e il cablaggio interno del modulo
- [ ] 2.3 Documentare la meccanica del pedale con trasmissione leva/settore per il potenziometro (D5)
- [ ] 2.4 Realizzare due moduli identici e verificare con multimetro che SIG resti tra 0 e 3,3 V su tutta la corsa

## 3. USB HID gamepad

- [ ] 3.1 Implementare classe `PedalsHID` (`USBHIDDevice`) con descrittore: Z, Rz, [Slider] a 16 bit + 8 pulsanti
- [ ] 3.2 Impostare nome prodotto "ESP32 Pedals & Shifter" e dispositivo composito HID + CDC
- [ ] 3.3 Implementare invio report on-change e ogni 10 ms, polling 1 ms
- [ ] 3.4 Verificare enumerazione su Windows in joy.cpl senza driver e senza assi fantasma

## 4. Acquisizione pedali

- [ ] 4.1 Implementare tick a 1 kHz (`esp_timer`) con lettura `analogReadMilliVolts` ×8 per porta
- [ ] 4.2 Implementare rilevamento presenza con conferma 50 ms e uscita a 0 se assente
- [ ] 4.3 Implementare rilevamento guasto fuori range per >100 ms
- [ ] 4.4 Implementare normalizzazione con calibrazione min/max e inversione
- [ ] 4.5 Implementare zone morte bassa/alta
- [ ] 4.6 Implementare curve lineare, esponenziale e tabella 5 punti
- [ ] 4.7 Implementare filtro EMA + isteresi e conversione a uint16
- [ ] 4.8 Scrivere test unitari (PlatformIO `native`) per normalizzazione, zone morte e curve

## 5. Pulsanti marcia

- [ ] 5.1 Implementare lettura GPIO 17/18 con pull-up e antirimbalzo a contatore (default 5 ms)
- [ ] 5.2 Implementare tenuta minima di 20 ms dello stato premuto nel report
- [ ] 5.3 Test unitari per antirimbalzo e tenuta minima

## 6. Configurazione

- [ ] 6.1 Definire struttura `Config` versionata con valori di default
- [ ] 6.2 Implementare caricamento/salvataggio NVS con fallback ai default
- [ ] 6.3 Implementare CLI su CDC: `help`, `show`, `set`, `save`, `reset`, `version`
- [ ] 6.4 Implementare calibrazione guidata `cal <port>` / `cal save` / `cal abort` con controllo corsa minima 10%
- [ ] 6.5 Implementare `monitor` a 10 Hz
- [ ] 6.6 Verificare che l'uso della CLI non interrompa i report HID

## 7. Integrazione Assetto Corsa e collaudo

- [ ] 7.1 Scrivere guida di mappatura in Assetto Corsa / Content Manager (`docs/assetto-corsa.md`) con preset di esempio
- [ ] 7.2 Collaudo in gioco: acceleratore e freno 0–100%, cambio marcia su/giù senza doppi cambi
- [ ] 7.3 Collaudo modularità: scambio moduli, scollegamento/ricollegamento a caldo, sostituzione modulo
- [ ] 7.4 Misurare rumore a pedale fermo (≤ ±0,5%) e frequenza report (≥ 500 Hz)
