#!/bin/bash

COMMAND="sudo systemctl stop visibility.service"

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

for HOST in "${HOSTS[@]}"; do
  echo "Starting $HOST"
  ssh -o ConnectTimeout=5 "$HOST" "$COMMAND" &
done

wait

echo "DONE"
