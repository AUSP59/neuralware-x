<!-- SPDX-License-Identifier: Apache-2.0 -->
# C++ Style Guide (Project-specific)

- C++17 minimum (unless otherwise specified).
- Prefer RAII, `std::unique_ptr`/`std::shared_ptr` over raw new/delete.
- Error handling: return `expected<T,E>` or `status` pattern; avoid exceptions in hot paths.
- Naming: `snake_case` for functions/variables, `PascalCase` for types, UPPER_SNAKE for macros.
- Headers: keep includes minimal; use forward declarations; treat headers as stable API.
- Concurrency: prefer `std::jthread`, `std::atomic`, and lock-free structures where safe.
- Performance: measure with benchmarks; avoid premature optimization.
- Security: avoid unbounded `sprintf`, prefer `snprintf` or C++ streams; validate inputs.
