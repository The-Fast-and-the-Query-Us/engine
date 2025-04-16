#!/bin/bash

mkdir -p frontend/build/
sudo apt install unzip
curl -fsSL https://bun.sh/install | bash
source ~/.bashrc
bun install
bun build --outdir frontend/build --target browser frontend/js/index.tsx
cp frontend/html/* frontend/build/
cp frontend/img/* frontend/build/
