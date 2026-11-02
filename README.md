
# zephyrrtos-mcpp

Lightweight, header-only C++ wrappers around Zephyr RTOS C APIs. This small library provides
chrono-friendly and RAII-style helpers for common Zephyr primitives:

- `tick_timer` — time_point/duration integration with `std::chrono`
- `this_thread` helpers (sleep/yield)
- counting and binary `semaphore`
- `message_queue` with blocking/timeout variants
- `poll` helpers and `signal`/`poll_event`
- `work` and `work_poll` wrappers

Headers are located in `include/zephyr/`. Licensed under Apache-2.0.

Usage: include the headers from your Zephyr C++ component and build as part of a Zephyr
project (no separate build step required).

## Development

The repository is a west manifest repository of its own only for test purpose.
Every header gets compiled on its own, to ensure that it is self-contained, and
clang-tidy (22 or later, configured by `.clang-tidy`) can check them as well:

```bash
west twister -T tests --integration
```

The clang-tidy scenario is tagged `clang_tidy`, select or exclude it with `--tag` / `--exclude-tag`.

To try a single header: `west build -b native_sim/native/64 tests/header_check -- -DUSE_CLANG_TIDY=ON`.
