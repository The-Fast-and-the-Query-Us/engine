#!/bin/bash

set -e

sudo systemctl stop query_server.service || true

cd src
cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build
make crawler VERBOSE=1
cd ../..

sudo cp systemd/crawler.service /etc/systemd/system/

username=$(whoami)
sudo sed -i "s/<USERNAME>/$username/g" /etc/systemd/system/crawler.service

sudo systemctl daemon-reload
sudo systemctl start crawler.service
sudo systemctl status crawler.service
