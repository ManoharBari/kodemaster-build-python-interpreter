#!/bin/bash
# Run the Python interpreter

set -e

# Navigate to project root
cd "$(dirname "$0")/.."

# Ensure compiled
if [ ! -f "./your_program" ]; then
    make
fi

# Run with arguments
./your_program "$@"
