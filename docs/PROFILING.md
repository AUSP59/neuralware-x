
# Profiling Playbook

- **Linux perf**: `perf record -- ./build/bin/nwx <args>` → `perf report`
- **gprof (GCC)**: compile with `-pg` → run → `gprof`.
- **Callgrind**: `valgrind --tool=callgrind ./build/bin/nwx` → kcachegrind.
- **BOLT (Clang)**: see `scripts/bolt_optimize.sh` after collecting `perf2bolt` data.
