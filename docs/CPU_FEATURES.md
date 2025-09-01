
# CPU Features & Build Flags

- Default builds target a conservative baseline.
- For max perf: `-march=native` (host-only) or specific `-mavx2 -mfma` if safe for your deployment.
- Consider runtime dispatch (if adding SIMD paths) and guard with CPUID checks.
