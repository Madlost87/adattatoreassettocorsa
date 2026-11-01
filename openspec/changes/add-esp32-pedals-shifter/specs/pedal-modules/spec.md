## ADDED Requirements

### Requirement: Modulo pedale indipendente
Ogni pedale (acceleratore, freno) SHALL essere un modulo meccanico ed elettrico separato dalla centralina, collegato tramite cavo e connettore staccabile. I due moduli MUST essere costruttivamente identici e intercambiabili; il ruolo (acceleratore o freno) SHALL dipendere solo dalla porta della centralina a cui è collegato.

#### Scenario: Scambio dei moduli
- **WHEN** il modulo collegato alla porta "THROTTLE" viene scambiato con quello della porta "BRAKE"
- **THEN** il sistema continua a funzionare dopo una nuova calibrazione, senza modifiche al firmware

#### Scenario: Sostituzione di un modulo guasto
- **WHEN** un modulo viene sostituito con uno nuovo dello stesso tipo
- **THEN** è sufficiente ricalibrare solo quella porta

### Requirement: Interfaccia elettrica standard del modulo
Ogni porta modulo MUST esporre un connettore a 4 poli con pinout fisso: 1 = 3V3, 2 = segnale (cursore potenziometro), 3 = GND, 4 = rilevamento presenza (collegato a GND all'interno del modulo). Il potenziometro SHALL essere lineare da 10 kΩ alimentato a 3,3 V (mai 5 V) e il segnale SHALL arrivare a un pin ADC1 dell'ESP32-S3.

#### Scenario: Tensione sul segnale entro i limiti
- **WHEN** il pedale viene premuto da tutto rilasciato a fine corsa
- **THEN** la tensione sul pin di segnale resta compresa tra 0 V e 3,3 V

### Requirement: Rilevamento presenza del modulo
Il firmware SHALL rilevare la presenza di ciascun modulo tramite il pin di rilevamento (ingresso con pull-up interno: LOW = presente). Con un modulo assente l'asse corrispondente MUST essere riportato a 0.

#### Scenario: Modulo scollegato durante l'uso
- **WHEN** il modulo freno viene scollegato con il dispositivo acceso
- **THEN** entro 50 ms l'asse freno viene riportato a 0
- **AND** la CLI seriale segnala "BRAKE: modulo assente"

#### Scenario: Modulo ricollegato
- **WHEN** il modulo viene ricollegato
- **THEN** l'asse riprende a funzionare usando la calibrazione salvata per quella porta

### Requirement: Rilevamento guasto del segnale
Il firmware SHALL considerare guasto un segnale che resta stabilmente fuori dall'intervallo calibrato oltre una tolleranza configurabile (es. cursore interrotto o cortocircuito) e MUST in tal caso riportare l'asse a 0.

#### Scenario: Cursore interrotto
- **WHEN** la lettura ADC rimane sotto (min_calibrato − tolleranza) o sopra (max_calibrato + tolleranza) per più di 100 ms
- **THEN** l'asse viene forzato a 0 e la CLI segnala un guasto per quella porta

### Requirement: Predisposizione per moduli aggiuntivi
La centralina SHALL prevedere una terza porta modulo (CLUTCH) con lo stesso pinout, non obbligatoria. Se il modulo non è presente il firmware MUST comportarsi come se la porta non esistesse lato gioco (asse a 0).

#### Scenario: Funzionamento senza frizione
- **WHEN** solo i moduli acceleratore e freno sono collegati
- **THEN** il dispositivo funziona normalmente e l'asse frizione resta a 0
