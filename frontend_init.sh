#!/bin/bash

bun install
bun build --outdir frontend/static --target browser frontend/js/index.tsx
mkdir -p frontend/build/
cp frontend/html/* frontend/build/
cp frontend/img/* frontend/build/
