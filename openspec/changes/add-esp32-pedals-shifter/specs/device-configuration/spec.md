## ADDED Requirements

### Requirement: Interfaccia seriale di configurazione
Il dispositivo SHALL esporre, sulla stessa connessione USB, una porta seriale CDC (dispositivo composito HID + CDC) con una CLI testuale a comandi, utilizzabile da qualsiasi terminale seriale su Windows, senza interferire con il funzionamento del gamepad.

#### Scenario: Comando di aiuto
- **WHEN** l'utente apre la porta COM e invia `help`
- **THEN** il dispositivo elenca tutti i comandi disponibili con una breve descrizione

#### Scenario: Gamepad attivo durante l'uso della CLI
- **WHEN** la CLI è aperta in un terminale
- **THEN** i report HID continuano a essere inviati regolarmente

### Requirement: Procedura di calibrazione guidata
La CLI SHALL offrire il comando `cal <throttle|brake|clutch>` che registra il minimo e il massimo grezzi mentre l'utente rilascia e preme a fondo il pedale, e `cal save` per confermarli. Durante la calibrazione l'asse interessato MUST essere riportato a 0 al gioco.

#### Scenario: Calibrazione dell'acceleratore
- **WHEN** l'utente esegue `cal throttle`, preme a fondo e rilascia il pedale, poi invia `cal save`
- **THEN** i nuovi min/max vengono applicati e salvati
- **AND** a pedale rilasciato l'uscita è 0 e a fondo corsa è 65535

#### Scenario: Calibrazione con corsa insufficiente
- **WHEN** la differenza tra max e min registrati è inferiore al 10% del fondo scala ADC
- **THEN** la calibrazione viene rifiutata con un messaggio di errore e restano attivi i valori precedenti

### Requirement: Parametri configurabili
La CLI SHALL permettere di leggere e impostare per ciascun asse: inversione, zone morte iniziale/finale, coefficiente filtro, curva di risposta; e per i pulsanti: tempo di antirimbalzo. Il comando `show` MUST stampare la configurazione corrente.

#### Scenario: Modifica della zona morta
- **WHEN** l'utente invia `set brake deadzone_low 5`
- **THEN** la zona morta iniziale del freno diventa 5% con effetto immediato

### Requirement: Persistenza in flash
La configurazione SHALL essere salvata in NVS con il comando `save` e MUST essere ricaricata all'avvio. La struttura salvata MUST includere un numero di versione; se assente o non valida il firmware SHALL caricare i valori di default.

#### Scenario: Riavvio dopo il salvataggio
- **WHEN** l'utente esegue `save` e poi scollega e ricollega il dispositivo
- **THEN** la configurazione salvata è di nuovo attiva

#### Scenario: Ripristino dei default
- **WHEN** l'utente invia `reset`
- **THEN** vengono ripristinati e salvati i valori di default

### Requirement: Diagnostica in tempo reale
La CLI SHALL offrire il comando `monitor` che stampa periodicamente (10 Hz) per ogni porta: presenza modulo, valore ADC grezzo, valore in uscita in percentuale e stato dei pulsanti, fino alla pressione di un tasto.

#### Scenario: Verifica cablaggio
- **WHEN** l'utente esegue `monitor` e preme il freno
- **THEN** il valore grezzo e la percentuale della porta BRAKE variano di conseguenza
