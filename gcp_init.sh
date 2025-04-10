sudo snap install cmake --classic
sudo apt update
sudo apt install g++-10 make libssl-dev -y
sudo cp ./crawler.service /etc/systemd/system/crawler.service
