#!/bin/bash

username=$(whoami)
sed -i "s/<USERNAME>/$username/g" systemd/*

sudo snap install cmake --classic
sudo apt update
sudo apt install g++-10 make libssl-dev -y
sudo cp systemd/* /etc/systemd/system/
sudo systemctl daemon-reload

sudo apt install unzip -y
curl -fsSL https://bun.sh/install | bash
