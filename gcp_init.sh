#!/bin/bash

sudo snap install cmake --classic
sudo apt update
sudo apt install g++-10 make libssl-dev -y
sudo cp ./crawler.service /etc/systemd/system/crawler.service
sudo systemctl daemon-reload

username=$(whoami)
sed -i "s/<USERNAME>/$username/g" "crawler.service"

curl -fsSL https://bun.sh/install | bash
