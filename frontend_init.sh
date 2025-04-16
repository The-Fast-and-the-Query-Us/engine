#!/bin/bash

sudo apt install unzip
curl -fsSL https://bun.sh/install | bash
source ~/.bashrc
bun install
bun build --outdir . --target browser frontend/js/index.tsx
cp frontend/static/* .
