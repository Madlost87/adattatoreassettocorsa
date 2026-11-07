# SETUP GAS — acceleratore manuale per Assetto Corsa

ESP32-S3 SuperMini USB-C: gamepad USB nativo (asse **X** = gas; pulsanti 1/2 = marcia su/giù) e COM CDC. Gamepad autonomo: app e Wi-Fi non sono richiesti per giocare. L’installer Windows viene creato nel repository e trasferito manualmente sul PC Windows.

## Cablaggio diretto

| Parte | Collegamento |
|---|---|
| linguetta laterale B50K | `3V3` |
| linguetta centrale/cursore | `GPIO4` |
| altra linguetta laterale | `GND` |
| marcia su (NA) | fra `GPIO5` e `GND` |
| marcia giù (NA) | fra `GPIO6` e `GND` |

Nessuna resistenza in serie, `5V` libero e massa comune. A componente scollegato, fra le due linguette laterali il multimetro deve leggere circa 50 kΩ. La centrale rispetto a una laterale varia ruotando l'albero.

## Meccanica

Presa su asse Z, albero su cuscinetti, corona 80 denti e pignone 20 (4:1): 60° alla presa = 240° al B50K. Centrare il potenziometro a metà corsa e lasciare margine agli estremi. Se la lettura ADC si appiattisce prima della battuta, ricentrare gli ingranaggi o ridurre la corsa della presa. Non erano presenti disegni nel repository; vedi [lista componenti](hardware/lista-acquisto.md).

## Build e primo flash Linux

La SuperMini rilevata monta un ESP32-S3 QFN56 con **4 MB** di flash. Prima del primo flash HID/CDC, il PHY USB interno deve essere assegnato a USB-OTG; di fabbrica può essere assegnato alla USB-JTAG (`303a:1001`). Verifica lo stato con:

```bash
espefuse --port /dev/ttyACM0 summary | rg -A 2 USB_PHY_SEL
```

Se indica che il PHY interno è assegnato al dispositivo USB anziché a USB-OTG, entra in bootloader (tieni premuto BOOT mentre ricolleghi USB) e imposta una sola volta l'eFuse **irreversibile**:

```bash
espefuse --port /dev/ttyACM0 --before no-reset --after no-reset burn-efuse USB_PHY_SEL
```

`DIS_USB_OTG` deve risultare `False` prima di procedere. Dopo questa modifica l'ESP32 usa USB-OTG all'avvio; per un flash manuale entra in bootloader con BOOT. Il comando evita RTS/DTR, che la USB-OTG non espone.

```bash
cd firmware
/usr/bin/pio run
./scripts/flash_first.sh /dev/ttyACM0
```

La tabella usa `ota_0` e `ota_1` da 2 031 616 byte, più `otadata` e NVS; il firmware deve entrare in ciascuno slot. `pio run` mostra la dimensione reale. Non esiste più una partizione o unità USB SETUP GAS.

## App Windows

L’app offline identifica `SETUP-GAS,1.0.0`, permette selezione manuale COM, mostra ADC, barra gas, pulsanti e contatori. Creazione installer (Windows, .NET 8 SDK e Inno Setup 6):

```powershell
cd app
.\build-installer.ps1
```

Questo deposita `release/Setup.exe`. Copiarlo manualmente sul PC Windows (chiavetta USB, rete locale o altro supporto) ed eseguirlo. OTA conserva NVS e non contiene l’installer.

Calibrazione: riposo → **Imposta 0%**; battuta → **Imposta 100%**; **Salva sull’ESP32**. Entrambi i versi sono accettati. Un intervallo sotto 300 conteggi ADC è rifiutato e resta l’ultima calibrazione. Senza calibrazione valida il gas è 0%, pulsanti attivi.

Cambio: Rapida=3 ms, Standard=5 ms, Più stabile=12 ms, Personalizzata=2–30. “Applica” cambia RAM, “Salva filtro” persiste. Lettura a 1 ms e debounce separato in pressione/rilascio; una pressione lunga è un solo evento.

## Protocollo COM (115200 8N1)

Test automatico Linux, senza `screen`:

```bash
python3 firmware/scripts/test_cdc.py /dev/ttyACM0
python3 firmware/scripts/test_cdc.py /dev/ttyACM0 "GET"
```

```text
ID?                  -> SETUP-GAS,1.0.0
GET                  -> ADC, gas, calibrazione, filtro, contatori
CAL ZERO | CAL FULL | CAL SAVE | CAL CLEAR
DEBOUNCE 2..30 | DEBOUNCE SAVE
COUNTERS CLEAR
```

## OTA e recupero

Al primo collegamento seriale inviare `WIFI SSID nome-rete`, `WIFI PASS segreto`, `OTA PASS password-ota`, `WIFI SAVE`, quindi riavviare. Le credenziali sono in NVS, mai in Git. Build sola: `cd firmware && PLATFORMIO_CORE_DIR=/tmp/setup-gas-pio pio run`. L’upload autenticato richiede password locale, non in Git:

```bash
cd firmware
printf '%s\n' 'PASSWORD_OTA' > .ota-password; chmod 600 .ota-password
python3 scripts/ota_update.py --ip 192.168.1.50
```

L’OTA usa lo slot inattivo e rollback al mancato avvio. Per recuperare, BOOT+RESET e primo flash via USB.

## Windows e Assetto Corsa

`Win + R` → `joy.cpl` → Proprietà: deve muoversi solo X; pulsanti 1/2. In **Assetto Corsa → Controlli**: Acceleratore=X, Marcia su=1, Marcia giù=2; lo sterzo resta sul volante.

## Verifiche eseguite

Su ESP32-S3 SuperMini reale sono stati verificati: flash completo 4 MB, eFuse USB-OTG, enumerazione USB (`303a:4005 SETUP GAS Acceleratore Assetto Corsa`), CDC (`SETUP-GAS,1.0.0`) e gamepad HID (`/dev/input/js0`, `event-joystick`, `hidraw`). Restano da verificare su Windows: calibrazione dall'app, `joy.cpl`, Assetto Corsa e OTA/Wi-Fi.
