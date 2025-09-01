#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
import json, subprocess, os, sys, tempfile, shutil, pathlib
THRESH = float(os.environ.get("BENCH_REGRESSION_MAX","10.0"))
def run_bench(gitref, out):
    subprocess.run(["git","checkout",gitref], check=False)
    subprocess.run(["cmake","-S","benchmarks","-B","build-bench","-G","Ninja"], check=False)
    subprocess.run(["cmake","--build","build-bench","-j2"], check=False)
    p = pathlib.Path("build-bench/nwx_bench")
    if p.exists():
        r = subprocess.run([str(p),"--benchmark_min_time=0.1","--benchmark_format=json"], capture_output=True, text=True)
        json.dump(json.loads(r.stdout), open(out,"w"))
    else:
        json.dump({"benchmarks":[]}, open(out,"w"))
def compare(a,b):
    ja, jb = json.load(open(a)), json.load(open(b))
    ma = {i["name"]: i for i in ja.get("benchmarks",[]) if "real_time" in i}
    mb = {i["name"]: i for i in jb.get("benchmarks",[]) if "real_time" in i}
    fails = []
    for name in sorted(set(ma)&set(mb)):
        ra = ma[name]["real_time"]
        rb = mb[name]["real_time"]
        if ra == 0: continue
        delta = (rb - ra) * 100.0 / ra
        if delta > THRESH:
            fails.append((name, delta))
    return fails
def main():
    prev = "HEAD~1"
    a,b = "bench_prev.json","bench_curr.json"
    run_bench(prev, a)
    run_bench("HEAD", b)
    fails = compare(a,b)
    for n,d in fails:
        print(f"Regression: {n}: +{d:.1f}%")
    if fails:
        sys.exit(2)
    print("Bench regressions within threshold")
if __name__ == "__main__":
    main()
