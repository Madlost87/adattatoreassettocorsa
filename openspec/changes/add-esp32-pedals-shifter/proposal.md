## Why

Per guidare in Assetto Corsa su PC Windows servono acceleratore e freno analogici e due comandi per cambiare marcia (su/giù). Le pedaliere commerciali costano e sono poco personalizzabili. Con un ESP32 economico, due potenziometri e due pulsanti si può costruire un controller fai-da-te, riconosciuto da Windows come normale periferica di gioco senza driver, con pedali modulari sostituibili o aggiornabili in modo indipendente.

## What Changes

- Nuovo firmware ESP32 (ESP32-S3, USB nativo) che si presenta a Windows come **gamepad USB HID** standard (DirectInput), quindi utilizzabile da Assetto Corsa senza driver né software aggiuntivi.
- Due **moduli pedale** (acceleratore e freno) fisicamente indipendenti, con potenziometro, collegati alla centralina tramite connettore standard a 3 poli + riconoscimento presenza; i moduli sono intercambiabili e sostituibili.
- Acquisizione analogica con sovracampionamento, filtro, calibrazione min/max, zona morta e curva di risposta configurabile per ciascun pedale.
- Due **pulsanti** per marcia su / marcia giù con antirimbalzo, esposti come pulsanti HID.
- Interfaccia di **configurazione via porta seriale USB** (calibrazione, zone morte, curve, diagnostica) con salvataggio permanente in memoria flash (NVS).
- Documentazione hardware (schema di collegamento, pinout, connettori) e guida alla mappatura dei comandi in Assetto Corsa.

## Capabilities

### New Capabilities
- `usb-hid-gamepad`: enumerazione USB come gamepad HID (2 assi + 2 pulsanti), frequenza di report, compatibilità Windows/Assetto Corsa.
- `pedal-modules`: interfaccia hardware modulare dei pedali (connettore, pinout, alimentazione, rilevamento presenza, intercambiabilità).
- `analog-pedal-input`: lettura, filtraggio, calibrazione, zona morta e curva di risposta degli assi acceleratore e freno.
- `gear-shift-buttons`: lettura dei pulsanti marcia su/giù con antirimbalzo e mappatura su pulsanti HID.
- `device-configuration`: CLI seriale di configurazione/diagnostica e persistenza dei parametri in NVS.

### Modified Capabilities
<!-- Nessuna: progetto nuovo, non esistono spec precedenti. -->

## Impact

- **Hardware**: scheda ESP32-S3 (es. ESP32-S3-DevKitC-1), 2 potenziometri lineari 10 kΩ, 2 pulsanti momentanei, connettori per i moduli pedale, cavo USB dati.
- **Firmware**: nuovo progetto PlatformIO (framework Arduino su ESP-IDF), libreria USB HID nativa di arduino-esp32 (TinyUSB).
- **PC**: Windows 10/11, nessun driver; configurazione dei comandi nel menu Controlli di Assetto Corsa (o Content Manager).
- **Fuori ambito ora**: volante/sterzo, force feedback, frizione (predisposta come terzo modulo opzionale), celle di carico.
