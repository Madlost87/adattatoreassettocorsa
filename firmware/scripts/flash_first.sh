#!/usr/bin/env bash
set -euo pipefail
port=${1:?Uso: $0 /dev/ttyACM0}
project="$(dirname "$0")/.."
pio_bin="${PIO_BIN:-/usr/bin/pio}"
"$pio_bin" run -d "$project"
build="$project/.pio/build/esp32-s3-supermini"
esptool --port "$port" --baud 460800 --before no-reset --after no-reset write-flash --flash-mode dio --flash-size 4MB \
  0x0 "$build/bootloader.bin" \
  0x8000 "$build/partitions.bin" \
  0xe000 "$build/ota_data_initial.bin" \
  0x20000 "$build/firmware.bin"
echo "Primo flash completato. Configura Wi-Fi via CDC: WIFI SSID <nome>; WIFI PASS <segreto>; WIFI SAVE"
