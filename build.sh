#!/bin/bash

# Set the project directory (adjust if necessary)
PROJECT_DIR=$(pwd)

# Set the build directory (you can change this to anything)
BUILD_DIR="$PROJECT_DIR/src/build"

# Check if build directory exists, if not, create it
if [ ! -d "$BUILD_DIR" ]; then
	  echo "Creating build directory..."
	    mkdir "$BUILD_DIR"
fi

# Navigate into the build directory
cd "$BUILD_DIR"

export CXX=/usr/bin/g++-10
export CC=/usr/bin/gcc-10

# Run cmake to configure the project
echo "Running cmake..."
cmake ..

# Build the project using make
echo "Building the project..."
make crawler visibility_server

