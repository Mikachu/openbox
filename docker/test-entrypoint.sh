#!/bin/bash
set -e

echo "Openbox Test Suite"
echo "=================="

# Start Xvfb
echo "Starting Xvfb..."
Xvfb :99 -screen 0 1920x1080x24 -ac +extension GLX &
XVFB_PID=$!
sleep 2

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    if [ -n "$XVFB_PID" ]; then
        kill $XVFB_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT

# Run tests based on arguments
case "${1:-all}" in
    unit)
        echo "Running unit tests..."
        meson test -C build --print-errorlogs
        ;;
    
    integration)
        echo "Running Python integration tests..."
        pytest tests/python/ -v --xvfb
        ;;
    
    property)
        echo "Running property-based tests..."
        pytest tests/python/test_property_based.py -v --hypothesis-seed=0
        ;;
    
    memory)
        echo "Running memory tests with Valgrind..."
        valgrind --error-exitcode=1 --leak-check=full \
            ./build/openbox/openbox --version
        ;;
    
    asan)
        echo "Running AddressSanitizer tests..."
        meson test -C build-asan --print-errorlogs
        ;;
    
    coverage)
        echo "Generating coverage report..."
        ninja -C build coverage
        gcovr -r . --html --html-details -o coverage.html
        ;;
    
    static)
        echo "Running static analysis..."
        cppcheck --error-exitcode=1 --enable=all -I. openbox/ obt/ obrender/
        ;;
    
    rust)
        echo "Running Rust tests..."
        cargo test --workspace
        cargo clippy --workspace -- -D warnings
        ;;
    
    all|*)
        echo "Running all tests..."
        
        echo "1. Unit tests..."
        meson test -C build --print-errorlogs || true
        
        echo "2. Python integration tests..."
        pytest tests/python/ -v --xvfb || true
        
        echo "3. Rust tests..."
        cargo test --workspace || true
        
        echo "4. Static analysis..."
        cppcheck --enable=all -I. openbox/ obt/ obrender/ 2>&1 | head -50 || true
        
        echo ""
        echo "Test suite complete!"
        ;;
esac
