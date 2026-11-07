#!/usr/bin/env python3
"""Authenticated OTA uploader. Password is read from a local untracked file."""
import argparse, base64, os, sys, time, urllib.request
p=argparse.ArgumentParser();p.add_argument("--ip",required=True);p.add_argument("--bin",default=".pio/build/esp32-s3-supermini/firmware.bin");p.add_argument("--password-file",default=".ota-password");a=p.parse_args()
if not os.path.isfile(a.bin): sys.exit("firmware.bin non trovato: eseguire pio run")
password=open(a.password_file).read().strip()
data=open(a.bin,"rb").read(); req=urllib.request.Request("http://%s/update"%a.ip,data=data,method="POST",headers={"Content-Type":"application/octet-stream","Authorization":"Basic "+base64.b64encode(("gas:"+password).encode()).decode(),"X-Gas-Version":"1.0.0"})
try:
 with urllib.request.urlopen(req,timeout=45) as r: print(r.read().decode())
 time.sleep(4)
 with urllib.request.urlopen("http://%s/version"%a.ip,timeout=15) as r: print("Versione dopo riavvio:",r.read().decode().strip())
except Exception as e: sys.exit("OTA fallito; lo slot attivo non e stato toccato: "+str(e))
