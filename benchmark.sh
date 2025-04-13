./reset_index.sh
cd src/build
make crawler
cd ../..
sudo systemctl start crawler.service
sleep 5
sudo systemctl stop crawler.service
sudo systemctl status crawler.service
