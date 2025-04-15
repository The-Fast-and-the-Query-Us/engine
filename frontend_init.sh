#!/bin/bash

mkdir -p frontend/build/

bun install
bun build --outdir frontend/build --target browser frontend/js/index.tsx
cp frontend/html/* frontend/build/
cp frontend/img/* frontend/build/
