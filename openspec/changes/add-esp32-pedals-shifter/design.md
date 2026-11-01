## Context

Progetto nuovo, senza codice esistente. Obiettivo: una centralina con ESP32 che legga due pedali a potenziometro (acceleratore, freno) e due pulsanti (marcia su/giù) e li presenti a un PC Windows come controller di gioco usabile in Assetto Corsa. Assetto Corsa legge le periferiche tramite DirectInput e permette di assegnare assi e pulsanti di dispositivi diversi a comandi diversi, quindi i pedali possono convivere con un volante o con la tastiera.

Vincoli:
- Nessun driver o software lato PC: deve funzionare "plug & play".
- Hardware economico e reperibile, costruibile con stampa 3D / legno.
- Pedali modulari: costruiti, collegati e sostituiti indipendentemente.

## Goals / Non-Goals

**Goals:**
- Gamepad USB HID con 2 assi a 16 bit (Z, Rz) + pulsanti, polling 1 ms.
- Moduli pedale identici e intercambiabili con connettore standard e rilevamento presenza.
- Lettura analogica stabile (sovracampionamento, filtro, calibrazione, zone morte, curve).
- Configurazione da porta seriale USB con salvataggio in flash.

**Non-Goals:**
- Sterzo / volante, force feedback, cambio ad H, freno a mano (eventuali change future).
- Celle di carico (load cell) sul freno: l'interfaccia modulo è predisposta per un futuro modulo "attivo", ma non è parte di questa change.
- App grafica di configurazione su PC (la CLI seriale è sufficiente).
- Connessione Bluetooth/wireless.

## Decisions

### D1. ESP32-S3 con USB nativo (non ESP32 classico)
L'ESP32-S3 ha un controller USB OTG e può enumerarsi direttamente come HID. 
- *Alternativa: ESP32 classico + BLE Gamepad* → scartata: latenza e jitter maggiori, accoppiamento Bluetooth, disconnessioni, consumo di risorse; ADC2 inutilizzabile con radio attiva.
- *Alternativa: ESP32 classico + seriale + vJoy sul PC* → scartata: richiede driver e un programma ponte sempre in esecuzione.
- Anche l'ESP32-S2 funzionerebbe (USB nativo); si sceglie S3 per diffusione delle devkit e per la possibilità di aggiungere funzioni future.
- Scheda di riferimento: **ESP32-S3-DevKitC-1** (usare la porta marcata "USB", non "UART").

### D2. Firmware Arduino-ESP32 su PlatformIO, descrittore HID personalizzato
Framework Arduino (core arduino-esp32 3.x, TinyUSB) gestito con PlatformIO: semplice da compilare e flashare. Build flag `ARDUINO_USB_MODE=0` (TinyUSB/OTG) e `ARDUINO_USB_CDC_ON_BOOT=1` per avere un dispositivo composito **HID + CDC**.
- La classe `USBHIDGamepad` integrata usa assi a 8 bit con segno: insufficiente. Si implementa una classe `USBHIDDevice` con descrittore proprio:
  - Usage Page Generic Desktop, Usage Gamepad
  - 3 assi 16 bit, logical 0–65535: Z (acceleratore), Rz (freno), Slider (frizione, opzionale)
  - 8 pulsanti (1 = marcia su, 2 = marcia giù, 3–8 riservati per estensioni)
- *Alternativa: ESP-IDF puro* → più controllo ma più complesso; non necessario.
- VID/PID: per uso personale si usa il default Espressif; nome prodotto "ESP32 Pedals & Shifter".
- Nota: con 3 assi dichiarati, Windows mostrerà sempre lo Slider; se si preferisce zero assi fantasma, la frizione è abilitabile a compile-time (`ENABLE_CLUTCH`), default disabilitata.

### D3. Pinout centralina (ESP32-S3-DevKitC-1)
Solo pin ADC1 per i segnali analogici (ADC2 condivide risorse con il Wi-Fi); evitati i pin di strapping (0, 3, 45, 46) e i pin USB (19, 20).

| Funzione              | GPIO | Note                       |
|-----------------------|------|----------------------------|
| THROTTLE segnale      | 4    | ADC1_CH3                   |
| BRAKE segnale         | 5    | ADC1_CH4                   |
| CLUTCH segnale (opz.) | 6    | ADC1_CH5                   |
| THROTTLE presenza     | 11   | INPUT_PULLUP, LOW=presente |
| BRAKE presenza        | 12   | INPUT_PULLUP               |
| CLUTCH presenza       | 13   | INPUT_PULLUP               |
| Marcia su             | 17   | INPUT_PULLUP, a GND        |
| Marcia giù            | 18   | INPUT_PULLUP, a GND        |
| LED stato             | 48 / 38 | RGB onboard (dipende dalla revisione della devkit) |

### D4. Interfaccia modulo pedale: connettore 4 poli
Connettore **GX12 a 4 poli** (aviazione, robusto, a vite) su pannello centralina; in alternativa JST-XH 4 poli per prototipi. Pinout: 1 = 3V3, 2 = SIG, 3 = GND, 4 = DETECT (ponticellato a GND nel modulo). Cavo schermato o twistato fino a ~2 m.
- Protezione sul lato centralina per ogni ingresso SIG: resistenza serie 1 kΩ + condensatore 100 nF verso GND (filtro RC anti-rumore/ESD, τ ≈ 0,1 ms).
- Il modulo contiene solo potenziometro lineare 10 kΩ (tipo B10K) e ponticello DETECT: nessuna elettronica attiva → economico e intercambiabile.
- Il contratto elettrico "0–3,3 V su SIG" permette di sostituire in futuro il potenziometro con un **sensore Hall lineare** (es. SS49E, magnete sul pedale) o con un modulo con amplificatore per cella di carico, senza cambiare centralina né firmware.
- *Alternativa: modulo con microcontrollore e bus I²C/RS485* → scartata ora: complessità non giustificata per due potenziometri.

### D5. Meccanica del potenziometro
Un pedale ruota tipicamente 20–40°, mentre un potenziometro rotativo ha ~270° di corsa: usando la rotazione diretta si sfrutta solo ~10–15% della scala (~400–600 passi ADC). Si raccomanda una **trasmissione con leva/settore dentato** (rapporto 1:3 – 1:6) per sfruttare almeno il 50% della corsa del potenziometro. La calibrazione per porta compensa qualunque corsa residua. Potenziometri a corsa lineare (slider) sono un'alternativa valida.

### D6. Pipeline di acquisizione
Loop a 1 kHz (timer hardware/`esp_timer`) per ogni porta:
1. Presenza modulo (DETECT) → se assente, uscita 0.
2. `analogReadMilliVolts()` ×8 (oversampling, corregge la non linearità dell'ADC tramite eFuse), attenuazione 11/12 dB.
3. Rilevamento guasto (fuori da [min−tol, max+tol] per >100 ms → 0).
4. Normalizzazione [min,max] → [0,1], inversione opzionale.
5. Zone morte (bassa/alta) e ri-scalatura su [0,1].
6. Curva di risposta (lineare / esponenziale / tabella 5 punti).
7. Filtro EMA + isteresi di ±1 LSB su scala 0–65535 (o soglia configurabile).
8. Conversione a uint16 e pubblicazione nello stato condiviso.

I pulsanti sono letti nello stesso tick (antirimbalzo a contatore, default 5 ms; tenuta minima 20 ms). Un task separato invia il report HID quando lo stato cambia o ogni 10 ms.

### D7. Configurazione: CLI seriale + NVS
La CLI gira in un task a bassa priorità su CDC. Struttura `Config { uint16 version; PortConfig ports[3]; uint8 debounce_ms; }` salvata come blob in NVS (`Preferences`). Versione errata → default. Comandi: `help`, `show`, `monitor`, `cal <port>`, `cal save`, `cal abort`, `set <port> <param> <value>`, `set buttons debounce <ms>`, `save`, `reset`, `version`.
- *Alternativa: web UI via Wi-Fi* → scartata: aggiunge complessità e interferenze con l'ADC; eventuale change futura.

### D8. Assetto Corsa
Nessuna integrazione specifica: il dispositivo è un normale controller DirectInput. In AC (o Content Manager → Impostazioni → Assetto Corsa → Controlli) si seleziona la modalità "Volante" e si assegnano Acceleratore → asse Z, Freno → asse Rz, Marcia su → pulsante 1, Marcia giù → pulsante 2. Se si usa un volante separato, si mappa lo sterzo sul volante e i pedali su questo dispositivo. La documentazione includerà le schermate/procedura e un preset `.ini` di esempio.

## Risks / Trade-offs

- [ADC ESP32 non lineare ai bordi (vicino a 0 V e 3,3 V)] → uso di `analogReadMilliVolts` con calibrazione eFuse, calibrazione min/max per porta e zone morte default 2%.
- [Rumore elettrico su cavi lunghi] → filtro RC in centralina, cavo schermato/twistato, oversampling ed EMA.
- [Usura del potenziometro (punti morti, gracchiare) col tempo] → rilevamento guasto + modulo sostituibile; upgrade a sensore Hall previsto dal contratto elettrico.
- [Collegamento a caldo dei GX12: i contatti non si connettono in ordine definito] → il firmware considera valido il modulo solo dopo 50 ms di DETECT stabile; SIG protetto da 1 kΩ. Raccomandato comunque scollegare a dispositivo spento.
- [Utente collega la porta "UART" invece di "USB" della devkit] → documentato chiaramente; il firmware stampa un avviso sulla UART all'avvio.
- [Windows memorizza in cache il descrittore HID per VID/PID] → in caso di modifiche al descrittore cambiare PID o disinstallare il dispositivo da Gestione dispositivi; documentato.
- [Corsa del potenziometro piccola → bassa risoluzione] → trasmissione meccanica (D5); il gioco percepisce comunque valori a 16 bit interpolati.

## Migration Plan

Non applicabile (primo rilascio). Rollback: riflashare la versione precedente del firmware via PlatformIO; la configurazione in NVS è versionata.

## Open Questions

- Pulsanti marcia: montati su leve (paddle) dietro un volante o su un pomello/cambio sequenziale? Non influisce sul firmware, solo sulla meccanica.
- Frizione da abilitare subito o lasciare disabilitata (default)?
- Serve un LED/feedback di stato visibile (es. lampeggio a modulo assente)?
