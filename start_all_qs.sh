#!/bin/bash

COMMAND="cd engine && git restore . && git fetch && git checkout main && git pull && ./query_init.sh"

HOSTS=(
  engine1
  engine2
  engine3
  sui1
  sui2
  sui3
  rose1
  rose2
  rose3
  jimmy1
  jimmy2
  jimmy3
  zur1
  zur2
  zur3
  lander1
  lander2
  lander3
)

ssh engine1 "sudo systemctl stop frontend.service"

for HOST in "${HOSTS[@]}"; do
  echo "Starting $HOST"
  ssh -o ConnectTimeout=5 "$HOST" "$COMMAND" &
done

wait

echo "qs started"

ssh engine1 "cd engine && ./frontend_init.sh"

echo "DONE!"


