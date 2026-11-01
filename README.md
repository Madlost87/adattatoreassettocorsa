# Adattatore Assetto Corsa – Pedaliera ESP32 + cambio sequenziale

Controller fai-da-te per **Assetto Corsa** su PC Windows: acceleratore e freno modulari a potenziometro e due pulsanti marcia su/giù, gestiti da un **ESP32-S3** che si presenta come gamepad USB HID (nessun driver richiesto).

## Struttura

- `openspec/` – specifiche (OpenSpec). Change attiva: [`add-esp32-pedals-shifter`](openspec/changes/add-esp32-pedals-shifter/)
  - [proposal](openspec/changes/add-esp32-pedals-shifter/proposal.md) · [design](openspec/changes/add-esp32-pedals-shifter/design.md) · [tasks](openspec/changes/add-esp32-pedals-shifter/tasks.md)
- `hardware/` – [lista d'acquisto](hardware/lista-acquisto.md), schemi (in arrivo)
- `firmware/` – firmware PlatformIO (in arrivo)

## Stato

Specifica completata, implementazione da iniziare.
