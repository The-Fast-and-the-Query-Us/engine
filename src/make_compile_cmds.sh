#!/bin/bash

# Run CMake to configure the project
cmake -B build

# Run compdb to generate compile_commands.json
if command -v compdb &> /dev/null; then
    compdb -p build/ list > compile_commands.json
    echo "compile_commands.json generated."
else
    echo "compdb not found. Install compdb to generate compile_commands.json."
fi
