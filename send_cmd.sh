local instances=(@a)

for instance in  "${instances}"; do
  echo "Starting $instance"
  ssh "$instance" "cd crawler && git pull && ./gcp_init && ./reset_index && ./frontend_init && sudo systemctl start crawler.service"
done
