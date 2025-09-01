#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
import os, sys, subprocess, json, pathlib
THRESH = float(os.environ.get("SIZE_GROWTH_MAX_PCT","15.0"))
def size(p): 
  try: return os.path.getsize(p)
  except: return 0
repo = pathlib.Path('.')
prev = subprocess.getoutput("git rev-parse HEAD~1")
if 'fatal' in prev:
  print("No previous commit; skipping"); sys.exit(0)
subprocess.run(["git","checkout","HEAD~1"], check=False)
subprocess.run(["cmake","-S",".","-B","build-prev","-G","Ninja","-DCMAKE_BUILD_TYPE=Release"], check=False)
subprocess.run(["cmake","--build","build-prev","-j2"], check=False)
subprocess.run(["git","checkout","-"], check=False)
subprocess.run(["cmake","-S",".","-B","build","-G","Ninja","-DCMAKE_BUILD_TYPE=Release"], check=True)
subprocess.run(["cmake","--build","build","-j2"], check=True)
prev_bin = next((str(p) for p in pathlib.Path("build-prev").rglob("nwx_app") if p.is_file()), "")
curr_bin = next((str(p) for p in pathlib.Path("build").rglob("nwx_app") if p.is_file()), "")
if not prev_bin or not curr_bin:
  print("Binary not found; skipping")
  sys.exit(0)
sp, sc = size(prev_bin), size(curr_bin)
if sp == 0: 
  print("Prev size 0; skipping"); sys.exit(0)
growth = (sc - sp) * 100.0 / sp
print(f"Size growth: {growth:.2f}% (prev={sp} bytes, curr={sc} bytes)")
if growth > THRESH:
  print(f"ERROR: growth exceeds {THRESH:.1f}%"); sys.exit(2)
print("OK: size growth within threshold")
