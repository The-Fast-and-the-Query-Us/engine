local instances=(@a)

for instance in  "${instances}"; do

  echo "Starting $instance"
  ssh "$instance" "cd engine && git restore . && git checkout main && git pull\
  && ./gcp_init && ./reset_index && ./frontend_init && sudo systemctl start
  crawler.service"

done
