# Openbox Testing Guide

## Overview

Openbox uses a comprehensive testing infrastructure including unit tests, integration tests, property-based tests, memory safety checks, and continuous integration.

## Quick Start

```bash
# Run all tests locally
./scripts/run-tests.sh

# Run specific test suite
pytest tests/python/test_integration.py -v

# Run with coverage
pytest tests/python/ --cov=openbox_dbus --cov-report=html
```

## Test Organization

```
tests/
├── python/              # Python integration tests
│   ├── conftest.py      # pytest fixtures and configuration
│   ├── test_integration.py     # X11 integration tests
│   ├── test_property_based.py  # Hypothesis property tests
│   └── test_memory_safety.py     # Valgrind/ASan tests
├── unit/                # C unit tests (using Check or similar)
├── rust/                # Rust crate tests
└── fixtures/            # Test fixtures and sample configs
```

## Running Tests

### Python Integration Tests

Requires: Python 3.8+, pytest, Xvfb

```bash
# Install dependencies
pip install -r tests/requirements.txt

# Run all Python tests
pytest tests/python/ -v

# Run with X11 display (headless)
pytest tests/python/ --xvfb

# Run specific test
pytest tests/python/test_integration.py::TestOpenboxLifecycle::test_openbox_version -v

# Run with coverage
pytest tests/python/ --cov=openbox_dbus --cov-report=html
```

### Property-Based Tests

Uses Hypothesis for generative testing:

```bash
# Run property-based tests
pytest tests/python/test_property_based.py -v

# Run with specific seed for reproducibility
pytest tests/python/test_property_based.py --hypothesis-seed=12345

# Run state machine tests
pytest tests/python/test_property_based.py::TestWindowManager -v
```

### Memory Safety Tests

Requires: Valgrind (optional), AddressSanitizer build

```bash
# Run Valgrind tests (if available)
pytest tests/python/test_memory_safety.py -v

# Build with AddressSanitizer
meson setup build-asan -Db_sanitize=address
ninja -C build-asan

# Run ASan tests
pytest tests/python/test_memory_safety.py::TestAddressSanitizer -v
```

### Rust Tests

```bash
# Run all Rust tests
cargo test --workspace

# Run with coverage
cargo tarpaulin --out Xml

# Run benchmarks
cargo bench
```

### C/C++ Unit Tests

```bash
# Build and run meson tests
meson test -C build --verbose

# Run with Valgrind
meson test -C build --wrap='valgrind --leak-check=full'

# Generate coverage
ninja -C build coverage
```

## Docker Testing

Use Docker for reproducible test environments:

```bash
# Build test image
docker build -t openbox:test -f docker/Dockerfile.test .

# Run all tests
docker run --rm openbox:test

# Run specific test suite
docker run --rm openbox:test unit
docker run --rm openbox:test integration
docker run --rm openbox:test memory
docker run --rm openbox:test asan
docker run --rm openbox:test coverage
```

## CI/CD Pipeline

GitHub Actions runs the following on every PR:

1. **Build Matrix**: GCC and Clang on Ubuntu 20.04/22.04
2. **Rust Build**: Stable and nightly Rust toolchains
3. **Static Analysis**: clang-tidy, cppcheck, clippy
4. **Memory Safety**: Valgrind, AddressSanitizer
5. **Python Tests**: pytest with Xvfb
6. **Coverage**: Codecov integration

### CI Status

[![CI](https://github.com/openbox/openbox/workflows/CI/badge.svg)](https://github.com/openbox/openbox/actions)

## Writing Tests

### Python Integration Test Example

```python
# tests/python/test_example.py
import pytest

@pytest.mark.x11
@pytest.mark.wm
def test_window_creation(xvfb_display, openbox):
    \"\"\"Test creating a window.\"\"\"
    # xvfb_display provides headless X11
    # openbox provides running window manager
    
    # Your test code here
    assert openbox.is_running()
```

### Property-Based Test Example

```python
from hypothesis import given, strategies as st

@given(
    x=st.integers(min_value=-10000, max_value=10000),
n    y=st.integers(min_value=-10000, max_value=10000),\n    width=st.integers(min_value=1, max_value=4096),\n    height=st.integers(min_value=1, max_value=4096)\n)
ndef test_geometry_valid(x, y, width, height):
n    \"\"\"Test that any valid geometry can be created.\"\"\"\n    assert width > 0\n    assert height > 0
```

## Test Markers

- `@pytest.mark.x11`: Requires X11 display
- `@pytest.mark.wm`: Requires window manager
- `@pytest.mark.dbus`: Requires D-Bus
- `@pytest.mark.slow`: Long-running tests (skip with `-m \"not slow\"`)
- `@pytest.mark.skipif`: Conditional skipping

## Coverage Reports

Coverage reports are generated in multiple formats:

- **HTML**: `htmlcov/` directory
- **XML**: `coverage.xml` for CI integration
- **Console**: Terminal output

View HTML report:

```bash
# After running tests with --cov
python -m http.server 8000 -d htmlcov/
# Open http://localhost:8000
```

## Debugging Tests

### Verbose Output

```bash
pytest tests/python/ -v --tb=long
```

### Stop on First Failure

```bash
pytest tests/python/ -x
```

### Run with GDB

```bash
gdb --args python -m pytest tests/python/test_integration.py -v
```

### X11 Debugging

```bash
# Run with visible X11 (for debugging)
pytest tests/python/test_integration.py --no-xvfb
```

## Continuous Integration

The CI pipeline includes:

| Job | Purpose | Artifacts |
|-----|---------|-----------|
| build-c | Build C/C++ with GCC/Clang | binaries |
| build-rust | Build Rust crates | binaries |
| static-analysis | clang-tidy, cppcheck, clippy | reports |
| asan | AddressSanitizer build | - |
| python-tests | pytest with Xvfb | coverage |
| docker | Reproducible Docker build | image |
| docs | Generate documentation | html |

## Troubleshooting

### Xvfb Issues

```bash
# Check Xvfb is installed
which Xvfb

# Manual Xvfb start
Xvfb :99 -screen 0 1920x1080x24 &
export DISPLAY=:99
```

### D-Bus Issues

```bash
# Start session bus
dbus-run-session -- pytest tests/python/
```

### Permission Issues

```bash
# Fix permissions for test artifacts
chmod -R u+w tests/fixtures/
```

## See Also

- [CI Configuration](../.github/workflows/ci.yml)
- [pytest Documentation](https://docs.pytest.org/)
- [Hypothesis Documentation](https://hypothesis.readthedocs.io/)
- [Codecov Integration](https://codecov.io/)
