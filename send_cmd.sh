for instance in  "$@"; do

  echo "Starting $instance"
  ssh "$instance" "cd engine && git restore . && git checkout main && git pull\
  && ./gcp_init.sh && ./reset_index.sh && ./frontend_init.sh && ./build.sh &&\
  sudo systemctl start crawler.service"

done
