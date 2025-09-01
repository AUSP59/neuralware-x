#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
import sys, subprocess, os, pathlib, re
# Bidi and zero-width characters to ban
BANNED = [
    '\u202A','\u202B','\u202D','\u202E','\u202C',
    '\u2066','\u2067','\u2068','\u2069',
    '\u200E','\u200F','\u061C',
    '\u200B','\u200C','\u200D','\uFEFF'
]
pattern = re.compile('|'.join(map(re.escape, BANNED)))
files = subprocess.check_output(["git","ls-files"]).decode().splitlines()
allow_dirs = {"site/","docs/site/",".git/","build/"}
failed = False
for f in files:
    if any(f.startswith(d) for d in allow_dirs): continue
    p = pathlib.Path(f)
    try:
        data = p.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        continue
    if pattern.search(data):
        print(f"ERROR: Trojan Source character found in {f}")
        failed = True
if failed:
    sys.exit(2)
print("Trojan Source scan OK")
