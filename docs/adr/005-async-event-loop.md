# ADR-005: Async/Tokio Event Loop

## Status

**Accepted** - 2024-03-01

## Context

Openbox 3.x uses a traditional select/poll-based event loop:

```c
while (running) {
    XNextEvent(display, &ev);
    handle_event(&ev);
}
```

This approach:

1. **Blocks on I/O**: Single-threaded, can't do work while waiting
2. **Callback hell**: Complex async operations require state machines
3. **No timeouts**: Timers require external libraries
4. **Difficult composition**: Multiple event sources are complex

Modern async runtimes solve these problems elegantly.

## Decision

We will **adopt Tokio** as our async runtime for the Rust core.

### Architecture

```rust
#[tokio::main]
async fn main() {
    let mut wm = WindowManager::new().await;
    
    // Spawn concurrent tasks
    tokio::spawn(x11_event_loop(wm.clone()));
    tokio::spawn(dbus_service(wm.clone()));
    tokio::spawn(config_watcher(wm.clone()));
    
    // Main control loop
    while wm.running().await {
        tokio::time::sleep(Duration::from_millis(16)).await;
    }
}
```

### Key Features

- **Async I/O**: Non-blocking X11, D-Bus, file operations
- **Tasks**: Lightweight concurrent execution
- **Channels**: Message passing between components
- **Timers**: Built-in timeout and interval support
- **Synchronization**: Mutexes, semaphores, etc.

## Consequences

### Positive

- **Performance**: Better CPU utilization
- **Responsiveness**: UI stays responsive during I/O
- **Composability**: Easy to combine multiple async operations
- **Ecosystem**: Rich crate ecosystem for async
- **Testing**: Async tests are straightforward

### Negative

- **Learning curve**: Developers must understand async/await
- **Debug complexity**: Stack traces can be confusing
- **Runtime overhead**: Small but present
- **Blocking operations**: Must be careful to not block runtime

### Migration

- C code uses blocking I/O (unchanged)
- Rust core uses async
- FFI boundary handles conversion

## Alternatives Considered

### Option A: Thread Pool
Use OS threads for concurrency.

- **Rejected**: Heavier than tasks, harder synchronization

### Option B: async-std
Alternative to Tokio.

- **Rejected**: Tokio has larger ecosystem, better integration

### Option C: Manual epoll/kqueue
Direct async I/O implementation.

- **Rejected**: Reinventing Tokio, maintenance burden

## Implementation

See [Async Patterns](../dev-guide/async-patterns.md).

## References

- [Tokio Documentation](https://tokio.rs/)
- [Async Rust Book](https://rust-lang.github.io/async-book/)
- [Pin and Suffering](https://fasterthanli.me/articles/pin-and-suffering)
