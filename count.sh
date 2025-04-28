#!/bin/bash

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

check() {
  if ! ssh $1 "systemctl status query_server.service | grep \"LOCKED\""; then
    echo "$1 NOT DONE"
  fi
}

for HOST in "${HOSTS[@]}"; do
  check $HOST &
done

wait

echo "DONE!"


