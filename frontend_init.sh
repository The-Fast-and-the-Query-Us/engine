#!/bin/bash

bun install
bun build --outdir . --target browser frontend/js/index.tsx
cp frontend/static/* .
