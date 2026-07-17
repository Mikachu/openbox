# Openbox Code Style Guide

This document defines coding standards for Openbox 4.

## Rust

We follow the [Rust Style Guide](https://doc.rust-lang.org/style-guide/) with these additions:

### Formatting

Always run `cargo fmt` before committing:

```bash
cargo fmt --all
```

### Linting

All code must pass Clippy:

```bash
cargo clippy --workspace -- -D warnings
```

Allowed exceptions (document in code):
```rust
#[allow(clippy::too_many_arguments)]  // Justified: X11 requires many parameters
fn create_window(...) { ... }
```

### Naming Conventions

| Item | Convention | Example |
|------|-----------|---------|
| Types | PascalCase | `WindowManager` |
| Functions | snake_case | `focus_window` |
| Variables | snake_case | `client_list` |
| Constants | UPPER_SNAKE_CASE | `MAX_DESKTOPS` |
| Modules | snake_case | `window_manager` |
| Traits | PascalCase | `Focusable` |
| Lifetimes | short, lowercase | `'a`, `'wm` |

### Documentation

All public APIs must have doc comments:

```rust
/// Focuses the specified window.
///
/// # Arguments
///
/// * `window_id` - The X11 window ID to focus
///
/// # Errors
///
/// Returns `WindowError::NotFound` if the window doesn't exist.
///
/// # Example
///
/// ```no_run
/// wm.focus_window(0x123456)?;
/// ```
pub fn focus_window(&self, window_id: WindowId) -> Result<(), WindowError> {
    // ...
}
```

### Error Handling

Use `thiserror` for error types:

```rust
use thiserror::Error;

#[derive(Error, Debug)]
pub enum WindowError {
    #[error("window not found: {0}")]
    NotFound(WindowId),
    #[error("permission denied: {0}")]
    PermissionDenied(String),
}
```

Use `?` operator, not `unwrap()` or `expect()` in production code.

### Async/Await

- Prefer async functions for I/O operations
- Mark blocking operations with `#[tokio::main]` or spawn blocking tasks
- Use channels (`tokio::sync::mpsc`) for communication between tasks

### Unsafe Code

Minimize unsafe code. When necessary:

```rust
// SAFETY: X11 connection is valid because...
unsafe { xlib::XMapWindow(display, window) }
```

Document every safety invariant.

## C

We follow the [Linux Kernel Style](https://www.kernel.org/doc/html/latest/process/coding-style.html) with modifications.

### Indentation

- 4 spaces (no tabs)
- 80 column limit (soft, 100 acceptable for complex conditions)

### Braces

K&R style:

```c
if (condition) {
    do_something();
} else {
    do_other();
}

// Functions always brace
int function(void)
{
    return 0;
}
```

### Naming

| Item | Convention | Example |
|------|-----------|---------|
| Functions | lowercase, underscore | `client_focus` |
| Types | lowercase, underscore + `_t` | `client_t` |
| Macros | UPPER_CASE | `MAX_CLIENTS` |
| Enums | UPPER_CASE | `CLIENT_STATE` |
| Globals | g_ prefix | `g_display` |

### Headers

Include guards:

```c
#ifndef OPENBOX_CLIENT_H
#define OPENBOX_CLIENT_H

/* ... */

#endif /* OPENBOX_CLIENT_H */
```

### Comments

C-style comments only:

```c
/* This is a comment */
/* Multi-line
   comment */

/* TODO: Refactor this function */
/* FIXME: Memory leak here */
/* NOTE: X11 quirk */
```

### Memory Management

- Check all allocations: `if (ptr == NULL) return NULL;`
- Free in reverse order of allocation
- Use `g_new0` (GLib) for zeroed allocation
- Document ownership in comments

## FFI (Rust ↔ C)

### C API Design

```c
/* opaque pointer pattern */
typedef struct ObWindowManager ob_wm_t;

/* constructor returns opaque pointer */
ob_wm_t *ob_wm_new(void);

/* methods take opaque pointer as first arg */
void ob_wm_focus_window(ob_wm_t *wm, uint32_t window_id);

/* destructor */
void ob_wm_free(ob_wm_t *wm);
```

### Rust FFI Bindings

```rust
#[repr(C)]
pub struct ObWindowManager {
    _opaque: [u8; 0],
}

extern "C" {
    pub fn ob_wm_new() -> *mut ObWindowManager;
    pub fn ob_wm_focus_window(wm: *mut ObWindowManager, window_id: u32);
    pub fn ob_wm_free(wm: *mut ObWindowManager);
}
```

### Safety

```rust
pub struct WindowManager {
    ptr: *mut ObWindowManager,
}

impl Drop for WindowManager {
    fn drop(&mut self) {
        unsafe { ob_wm_free(self.ptr) }
    }
}
```

## Documentation

### Markdown

- Use ATX-style headers (`#` not `===`)
- Hard wrap at 80 columns for readability
- Use reference-style links for readability

### Man Pages

Use scdoc format, converted to Markdown:

```scdoc
OPENBOX(1)

# NAME

openbox - A highly configurable window manager

# SYNOPSIS

*openbox* [options]

# DESCRIPTION

Openbox is...
```

## Git Commit Messages

```
<type>(<scope>): <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style (formatting)
- `refactor`: Code change, no behavior change
- `test`: Tests
- `chore`: Build, CI, etc.

Examples:

```
feat(wm): add window snapping

Implements magnetic edges for windows near screen edges.
Holding Shift disables snapping.

Closes #123
```

```
fix(render): correct shadow offset calculation

Shadows were offset by 1px due to integer truncation.

Fixes #456
```

## CI Checks

All PRs must pass:

1. **Rust**: `cargo fmt --check`, `cargo clippy -- -D warnings`, `cargo test`
2. **C**: `clang-format`, `cppcheck`, `clang-tidy`
3. **Tests**: All unit and integration tests
4. **Documentation**: `mkdocs build`

## Editor Configuration

### VS Code

Install extensions:
- rust-analyzer
- C/C++
- markdownlint
- Code Spell Checker

Settings:

```json
{
    "editor.formatOnSave": true,
    "rust-analyzer.checkOnSave.command": "clippy",
    "C_Cpp.clang_format_style": "{IndentWidth: 4, ColumnLimit: 80}"
}
```

### Vim/Neovim

Use `rust.vim`, `coc.nvim` or `nvim-lspconfig`.

### Emacs

Use `rust-mode`, `lsp-mode`.

## Questions?

Ask in [GitHub Discussions](https://github.com/openbox/openbox/discussions) or [Discord](https://discord.gg/openbox).
