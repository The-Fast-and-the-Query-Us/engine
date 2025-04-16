for instance in  "$@"; do

  echo "Starting $instance"
  ssh "$instance" "(git clone\
  git@github.com:The-Fast-and-the-Query-Us/engine.git || true) && cd engine &&\
  git restore . && git checkout main && git pull && ./gcp_init.sh &&\
  sudo systemctl stop crawler.service && sudo systemctl stop visibility.service &&\
  ./reset_index.sh && ./frontend_init.sh && ./build.sh &&\
  sudo systemctl start crawler.service && sudo systemctl start visibility.service &&\
  sudo systemctl status crawler.service && sudo systemctl status visibility.service"

done
