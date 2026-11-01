## ADDED Requirements

### Requirement: Enumerazione come gamepad USB HID
Il dispositivo SHALL enumerarsi su Windows 10/11 come periferica di gioco USB HID standard (Generic Desktop / Gamepad), senza richiedere driver aggiuntivi, con nome prodotto "ESP32 Pedals & Shifter".

#### Scenario: Collegamento a un PC Windows
- **WHEN** l'ESP32-S3 viene collegato via USB alla porta nativa (OTG) di un PC Windows
- **THEN** il dispositivo compare in "Configura controller di gioco USB" (joy.cpl) entro 5 secondi
- **AND** non viene richiesta l'installazione di alcun driver

### Requirement: Layout del report HID
Il report HID MUST contenere: asse acceleratore, asse freno (entrambi a 16 bit senza segno, 0–65535, su usage distinti Z e Rz) e almeno 2 pulsanti (pulsante 1 = marcia su, pulsante 2 = marcia giù). L'asse frizione (Slider, 16 bit) SHALL essere dichiarato solo se abilitato in compilazione; nessun altro asse SHALL essere dichiarato, per evitare assi fantasma in gioco.

#### Scenario: Assi visibili in joy.cpl
- **WHEN** l'utente apre le proprietà del dispositivo in joy.cpl e preme i pedali
- **THEN** due assi distinti si muovono in modo indipendente su tutta la corsa
- **AND** nessun altro asse risulta presente

#### Scenario: Pedale a riposo
- **WHEN** entrambi i pedali sono rilasciati
- **THEN** il valore riportato per entrambi gli assi è 0

### Requirement: Frequenza e latenza dei report
Il firmware SHALL inviare report HID con intervallo di polling di 1 ms (1000 Hz) e MUST inviare un report almeno ogni 10 ms anche senza variazioni. La latenza tra variazione fisica e report SHALL essere inferiore a 5 ms (escluso il filtro configurato).

#### Scenario: Misura della frequenza
- **WHEN** si misura la cadenza dei report con uno strumento di analisi USB o test di input
- **THEN** la frequenza media è ≥ 500 Hz

### Requirement: Compatibilità con Assetto Corsa
Il dispositivo SHALL poter essere assegnato nel menu Controlli di Assetto Corsa (e Content Manager) ai comandi Acceleratore, Freno, Marcia su e Marcia giù, anche in combinazione con altre periferiche (volante, tastiera).

#### Scenario: Mappatura in gioco
- **WHEN** l'utente in Assetto Corsa → Opzioni → Controlli clicca su "Acceleratore" e preme il pedale
- **THEN** il gioco rileva l'asse corretto del dispositivo
- **AND** lo stesso vale per Freno (asse) e per Marcia su / Marcia giù (pulsanti)
