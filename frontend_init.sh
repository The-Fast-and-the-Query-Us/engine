#!/bin/bash

sudo systemctl stop frontend.service

cd src
cmake -B build
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
