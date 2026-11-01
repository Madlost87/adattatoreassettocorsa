## ADDED Requirements

### Requirement: Pulsanti marcia su e marcia giù
Il dispositivo SHALL avere due pulsanti momentanei normalmente aperti, collegati tra un GPIO e GND con pull-up interno attivo. Il pulsante "marcia su" MUST corrispondere al pulsante HID 1 e "marcia giù" al pulsante HID 2.

#### Scenario: Pressione di marcia su
- **WHEN** l'utente preme il pulsante marcia su
- **THEN** il pulsante HID 1 risulta premuto finché il pulsante fisico è tenuto premuto

#### Scenario: Pressione simultanea
- **WHEN** entrambi i pulsanti sono premuti insieme
- **THEN** entrambi i pulsanti HID risultano premuti (nessun blocco reciproco lato firmware)

### Requirement: Antirimbalzo
Il firmware SHALL applicare un antirimbalzo con tempo configurabile (default 5 ms) e MUST garantire che una singola pressione fisica produca una sola transizione premuto/rilasciato nel report HID.

#### Scenario: Contatto che rimbalza
- **WHEN** il contatto del pulsante rimbalza per 3 ms alla pressione
- **THEN** il gioco registra un solo cambio marcia

### Requirement: Durata minima della pressione riportata
Il firmware SHALL mantenere lo stato "premuto" nel report HID per almeno 20 ms anche per pressioni fisiche più brevi, così che il gioco non perda il comando.

#### Scenario: Colpetto rapido sul paddle
- **WHEN** il pulsante viene premuto per 8 ms
- **THEN** il pulsante HID resta premuto per almeno 20 ms e Assetto Corsa esegue il cambio marcia
