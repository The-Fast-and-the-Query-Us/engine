./reset_index.sh
cd src/build
make crawler
cd ../..
sudo systemctl start crawler.service
sleep 60
sudo systemctl stop crawler.service
sudo systemctl status crawler.service
