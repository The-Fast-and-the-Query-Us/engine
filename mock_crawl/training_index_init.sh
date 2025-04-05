#!/bin/bash
rm -rf ~/.local/share/crawler/training/index/** ~/.local/share/crawler/training/index_chunk
mkdir -p ~/.local/share/crawler/training
mkdir -p ~/.local/share/crawler/training/index
echo "Training index cleared"
