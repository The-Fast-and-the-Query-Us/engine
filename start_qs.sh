#!/bin/bash

COMMAND="cd engine && git restore . && git fetch && git checkout ranker_final && git pull && ./query_init.sh"

for HOST in "$@"; do
  echo "Starting $HOST"
  ssh -o BatchMode=yes -o ConnectTimeout=5 "$HOST" "$COMMAND"
done
