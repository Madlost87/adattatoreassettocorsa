## ADDED Requirements

### Requirement: Acquisizione con sovracampionamento
Il firmware SHALL campionare ciascun pedale con l'ADC a 12 bit dell'ESP32-S3 usando la lettura calibrata in millivolt (curva di calibrazione eFuse) e MUST mediare almeno 8 campioni per ciascun valore prodotto, con frequenza di aggiornamento per asse ≥ 1 kHz.

#### Scenario: Rumore a pedale fermo
- **WHEN** il pedale è fermo in una posizione qualsiasi
- **THEN** il valore HID in uscita (dopo filtro) oscilla al massimo di ±0,5% del fondo scala

### Requirement: Filtro configurabile
Il firmware SHALL applicare un filtro passa-basso esponenziale (EMA) per asse, con coefficiente configurabile, e una isteresi minima sul valore di uscita per eliminare il tremolio.

#### Scenario: Filtro disattivato
- **WHEN** il coefficiente del filtro è impostato a 1,0
- **THEN** il valore in uscita segue direttamente la media dei campioni senza ulteriore ritardo

### Requirement: Calibrazione per porta
Ogni porta pedale SHALL avere valori di calibrazione propri (min e max grezzi) e l'uscita MUST essere normalizzata linearmente da [min, max] a [0, 65535] con saturazione ai limiti. Il verso del pedale SHALL essere invertibile via configurazione.

#### Scenario: Normalizzazione
- **WHEN** la lettura è pari al minimo calibrato
- **THEN** l'uscita è 0
- **WHEN** la lettura è pari o superiore al massimo calibrato
- **THEN** l'uscita è 65535

#### Scenario: Potenziometro montato al contrario
- **WHEN** l'opzione "invert" è attiva per una porta
- **THEN** il pedale rilasciato produce 0 e premuto produce 65535 anche se la tensione decresce con la pressione

### Requirement: Zone morte
Ogni asse SHALL avere una zona morta iniziale e una finale configurabili in percentuale (default 2% e 2%). Sotto la zona morta iniziale l'uscita MUST essere 0; oltre la zona morta finale l'uscita MUST essere 65535.

#### Scenario: Piede appoggiato sul pedale
- **WHEN** il piede appoggiato muove il pedale meno della zona morta iniziale
- **THEN** l'asse riporta 0

#### Scenario: Pedale a fondo corsa
- **WHEN** il pedale entra nella zona morta finale
- **THEN** l'asse riporta 65535 (100%)

### Requirement: Curva di risposta
Ogni asse SHALL supportare una curva di risposta configurabile: lineare (default), esponenziale con fattore regolabile, oppure tabella a 5 punti interpolata linearmente.

#### Scenario: Curva progressiva sul freno
- **WHEN** il freno è configurato con curva esponenziale e il pedale è al 50% della corsa
- **THEN** l'uscita è inferiore al 50% secondo la formula della curva configurata
