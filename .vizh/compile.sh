#!/bin/bash
# Compile the Python interpreter

set -e

# Navigate to project root
cd "$(dirname "$0")/.."

# Clean and rebuild
make clean 2>/dev/null || true
make

echo "Compilation successful!"
