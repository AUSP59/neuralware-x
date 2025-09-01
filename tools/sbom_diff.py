#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
import json, sys
a, b = sys.argv[1], sys.argv[2]
ja, jb = json.load(open(a)), json.load(open(b))
def pkgs(j): return {(p.get("name",""), p.get("versionInfo","")) for p in j.get("packages",[])}
A, B = pkgs(ja), pkgs(jb)
added = sorted(list(B - A))
removed = sorted(list(A - B))
print("Added packages:", len(added))
for n,v in added[:50]:
    print(" +", n, v)
print("Removed packages:", len(removed))
for n,v in removed[:50]:
    print(" -", n, v)
# Fail if too many new deps (heuristic)
if len(added) > 10:
    print("ERROR: Too many new dependencies introduced (>10)")
    sys.exit(2)
