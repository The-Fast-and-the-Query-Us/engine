#!/bin/bash

set -e

sudo systemctl stop frontend.service || true

cd src
cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build
make server
cd ../..

sudo cp systemd/frontend.service /etc/systemd/system/

username=$(whoami)
sudo sed -i "s/<USERNAME>/$username/g" /etc/systemd/system/frontend.service

sudo systemctl daemon-reload
sudo systemctl enable frontend.service
sudo systemctl start frontend.service
sudo systemctl status frontend.service
