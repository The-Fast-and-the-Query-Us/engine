./reset_index.sh
cd src/build
make crawler
cd ../..

# add prompt there
read -p "Press Enter to start the crawler..."

sudo systemctl start crawler.service
sleep 60
sudo systemctl stop crawler.service
sudo systemctl status crawler.service
