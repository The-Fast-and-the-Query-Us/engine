#!/bin/bash

COMMAND="cd engine && git restore . && git fetch && git checkout html && git pull && ./crawler_init.sh"

# start all crawlers except for engine1
HOSTS=(
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

for HOST in "${HOSTS[@]}"; do
  echo "Starting $HOST"
  ssh -o ConnectTimeout=5 "$HOST" "$COMMAND" &
done

wait

echo "DONE"
