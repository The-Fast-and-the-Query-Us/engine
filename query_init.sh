#!/bin/bash

sudo systemctl stop query_server.service

cd src
cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build
make query_server VERBOSE=1
cd ../..

sudo cp systemd/query_server.service /etc/systemd/system/

username=$(whoami)
sudo sed -i "s/<USERNAME>/$username/g" /etc/systemd/system/query_server.service

sudo systemctl daemon-reload
sudo systemctl enable query_server.service
sudo systemctl start query_server.service
sudo systemctl status query_server.service
