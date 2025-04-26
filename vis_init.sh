#!/bin/bash

set -e

sudo systemctl stop visibility.service || true

cd src
cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build
make visibility_server VERBOSE=1
cd ../..

sudo cp systemd/visibility.service /etc/systemd/system/

username=$(whoami)
sudo sed -i "s/<USERNAME>/$username/g" /etc/systemd/system/visibility.service

sudo systemctl daemon-reload
sudo systemctl start visibility.service
sudo systemctl status visibility.service
