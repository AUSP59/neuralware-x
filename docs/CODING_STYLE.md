
# C++ Coding Style

- Follows `.clang-format` (LLVM base).
- `#pragma once` in headers; limit public APIs to `include/`.
- Use `std::unique_ptr`/`std::shared_ptr`; no raw `new` in high-level code.
- Prefer `span`/`string_view` and `gsl::not_null` semantics where applicable.
- Error handling: status-or pattern or exceptions; never silently ignore errors.
