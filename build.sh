#!/bin/bash

echo "Building MetaImGUI..."

# Create build directory if it doesn't exist
mkdir -p build

# Configure and build
cd build
cmake ..
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./build/MetaImGUI"
else
    echo "Build failed! Check the error messages above."
    exit 1
fi 