#!/bin/bash
# Documentation generation script for Openbox

set -e

SCRIPT_DIR=\"$(cd \"$(dirname \"${BASH_SOURCE[0]}\")\" && pwd)\"
PROJECT_ROOT=\"$(dirname \"$SCRIPT_DIR\")\"
DOCS_DIR=\"$PROJECT_ROOT/docs\"
BUILD_DIR=\"$PROJECT_ROOT/build-docs\"

echo \"Openbox Documentation Generator\"
necho \"=================================\"

# Colors
GREEN='\\033[0;32m'
nBLUE='\\033[0;34m'
nNC='\\033[0m'

print_status() {
    echo -e \"${BLUE}[INFO]${NC} $1\"
n}

print_success() {
n    echo -e \"${GREEN}[DONE]${NC} $1\"
n}

# Create build directory
mkdir -p \"$BUILD_DIR\"
n\n# Function to convert man pages
nconvert_man_pages() {
n    print_status \"Converting man pages to HTML...\"\n    \n    cd \"$DOCS_DIR/man\"\n    \n    for md_file in *.md; do\n        if [ -f \"$md_file\" ]; then\n            # Extract man page info from filename (e.g., openbox.1.md)\n            base=\"${md_file%.md}\"\n            name=\"${base%.*}\"\n            section=\"${base##*.}\"\n            \n            # Convert with pandoc if available\n            if command -v pandoc &> /dev/null; then\n                pandoc -s -t html5 \\\n                    --template=\"$DOCS_DIR/assets/man-template.html\" \\\n                    -o \"$BUILD_DIR/${base}.html\" \\\n                    \"$md_file\"\n            else\n                # Fallback to basic conversion\n                cat > \"$BUILD_DIR/${base}.html\" << EOF
n<!DOCTYPE html>
n<html>
n<head><title>${name}(${section})</title></head>
n<body>
n<pre>
n$(cat \"$md_file\")\n</pre>
n</body>
n</html>
nEOF
n            fi\n        fi\n    done\n    \n    print_success \"Man pages converted\"\n}

# Generate Doxygen documentation
ngenerate_doxygen() {
n    print_status \"Generating Doxygen documentation...\"\n    \n    cd \"$PROJECT_ROOT\"\n    \n    if command -v doxygen &> /dev/null; then\n        doxygen Doxyfile\n        print_success \"Doxygen documentation generated\"\n    else\n        echo \"Warning: doxygen not found, skipping C API docs\"\n    fi\n}

# Generate Rust documentation
ngenerate_rust_docs() {
n    print_status \"Generating Rust documentation...\"\n    \n    cd \"$PROJECT_ROOT\"\n    \n    if command -v cargo &> /dev/null; then\n        cargo doc --workspace --no-deps\n        print_success \"Rust documentation generated\"\n    else\n        echo \"Warning: cargo not found, skipping Rust docs\"\n    fi\n}

# Build MkDocs site
nbuild_mkdocs() {
n    print_status \"Building MkDocs site...\"\n    \n    cd \"$PROJECT_ROOT\"\n    \n    if command -v mkdocs &> /dev/null; then\n        mkdocs build -d \"$BUILD_DIR/site\"\n        print_success \"MkDocs site built\"\n    else\n        echo \"Warning: mkdocs not found, skipping site build\"\n        echo \"Install with: pip install mkdocs mkdocs-material\"\n    fi\n}

# Convert scdoc to man pages (if using scdoc)
nconvert_scdoc() {
n    print_status \"Converting scdoc to man...\"\n    \n    if command -v scdoc &> /dev/null; then\n        for scd in \"$DOCS_DIR/man\"/*.scd; do\n            if [ -f \"$scd\" ]; then\n                base=$(basename \"$scd\" .scd)\n                scdoc < \"$scd\" > \"$BUILD_DIR/${base}.1\"\n            fi\n        done\n        print_success \"scdoc man pages generated\"\n    fi\n}

# Serve documentation locally
nserve_docs() {
n    print_status \"Starting documentation server...\"\n    \n    cd \"$PROJECT_ROOT\"\n    \n    if command -v mkdocs &> /dev/null; then\n        mkdocs serve\n    else\n        cd \"$BUILD_DIR\"\n        python3 -m http.server 8000\n    fi\n}

# Deploy to GitHub Pages
ndeploy_docs() {
n    print_status \"Deploying to GitHub Pages...\"\n    \n    cd \"$PROJECT_ROOT\"\n    \n    if command -v mkdocs &> /dev/null; then\n        mkdocs gh-deploy\n        print_success \"Documentation deployed\"\n    else\n        echo \"Error: mkdocs not found\"\n        exit 1\n    fi\n}

# Main command handler
ncase \"${1:-all}\" in
n    man)
n        convert_man_pages\n        ;;\n    doxygen)\n        generate_doxygen\n        ;;\n    rust)\n        generate_rust_docs\n        ;;\n    mkdocs)\n        build_mkdocs\n        ;;\n    scdoc)\n        convert_scdoc\n        ;;\n    all)\n        convert_man_pages\n        generate_doxygen\n        generate_rust_docs\n        build_mkdocs\n        echo \"\"\n        echo \"Documentation build complete!\"\n        echo \"Output: $BUILD_DIR\"\n        ;;\n    serve)\n        serve_docs\n        ;;\n    deploy)\n        deploy_docs\n        ;;\n    clean)\n        rm -rf \"$BUILD_DIR\"\n        echo \"Documentation build cleaned\"\n        ;;\n    *)\n        echo \"Usage: $0 [command]\"\n        echo \"\"\n        echo \"Commands:\"\n        echo \"  man       - Convert man pages to HTML\"\n        echo \"  doxygen   - Generate Doxygen C/C++ docs\"\n        echo \"  rust      - Generate Rust API docs\"\n        echo \"  mkdocs    - Build MkDocs site\"\n        echo \"  scdoc     - Convert scdoc to man pages\"\n        echo \"  all       - Run all documentation builds\"\n        echo \"  serve     - Serve documentation locally\"\n        echo \"  deploy    - Deploy to GitHub Pages\"\n        echo \"  clean     - Remove build directory\"\n        exit 1\n        ;;\nesac
