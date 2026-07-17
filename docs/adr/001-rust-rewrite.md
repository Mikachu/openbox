# ADR-001: Rewrite Core in Rust

## Status

**Accepted** - 2024-01-15

## Context

Openbox 3.x is written entirely in C, which has served us well for nearly 20 years. However, we've encountered several recurring issues:

1. **Memory safety bugs**: Use-after-free, buffer overflows, and memory leaks are the most common bug reports
2. **Concurrency challenges**: X11 event handling is complex and error-prone in C
3. **Maintenance burden**: New contributors struggle with manual memory management
4. **Modern expectations**: Users expect crash-free experiences

We need a path forward that preserves our investment in C code while gaining modern safety guarantees.

## Decision

We will **rewrite the Openbox core in Rust** while maintaining C API compatibility for:

- Theme rendering (obrender)
- Utility library (obt)
- Plugin interface

### Architecture

```
┌─────────────────────────────────────┐
│         C API Layer                 │  ← Plugins, themes
│    (FFI compatibility maintained)     │
├─────────────────────────────────────┤
│         Rust Core                   │
│  ┌─────────┐ ┌─────────┐ ┌──────┐ │
│  │ Client  │ │ Focus   │ │Stack │  │
│  │ Manager │ │ Tracker │ │ Mgr  │  │
│  └─────────┘ └─────────┘ └──────┘ │
│  ┌─────────┐ ┌─────────┐           │
│  │ Event   │ │ Config  │           │
│  │ Dispatch│ │ Manager │           │
│  └─────────┘ └─────────┘           │
├─────────────────────────────────────┤
│         Platform Layer              │
│    (X11/Wayland backends)           │
└─────────────────────────────────────┘
```

## Consequences

### Positive

- **Memory safety**: Eliminates entire classes of bugs at compile time
- **Better concurrency**: Rust's ownership model enables safe parallelism
- **Modern tooling**: Cargo, Clippy, rustfmt improve developer experience
- **Performance**: Zero-cost abstractions, often faster than C
- **Ecosystem**: Access to crates.io for parsing, async, etc.

### Negative

- **Learning curve**: Contributors must learn Rust
- **Build complexity**: Dual C/Rust build system
- **Binary size**: Slightly larger binaries (mitigated by LTO)
- **Debugging**: Different tools than traditional C debugging

### Neutral

- **FFI overhead**: Minimal with proper design (benchmarked <1%)
- **Compile times**: Acceptable with incremental compilation

## Alternatives Considered

### Option A: Improve C Code
Continue with C, add more static analysis and testing.

- **Rejected**: Doesn't solve fundamental memory safety issues

### Option B: Use C++
Modern C++ with smart pointers and RAII.

- **Rejected**: Still has memory safety pitfalls, complex language

### Option C: Incremental Rewrite
Gradually replace C modules with Rust.

- **Accepted**: This is our actual approach, module by module

## Implementation

See [Rust Core Documentation](../dev-guide/rust-core.md) for implementation details.

## References

- [Rust Book](https://doc.rust-lang.org/book/)
- [Rust FFI Guide](https://doc.rust-lang.org/nomicon/ffi.html)
- [Benchmarks](../dev-guide/benchmarks.md)
