#!/bin/bash

mkdir -p frontend/build/
sudo apt install unzip -y
curl -fsSL https://bun.sh/install | bash
BUN="$HOME/.bun/bin/bun"
$BUN install
$BUN build --outdir frontend/build --target browser frontend/js/index.tsx
cp frontend/html/* frontend/build/
cp frontend/img/* frontend/build/
