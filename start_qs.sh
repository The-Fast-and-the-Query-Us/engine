#!/bin/bash

COMMAND="cd engine && git restore . && git checkout main && git pull && ./query_init"

for HOST in "$@"; do
  echo "Starting $HOST"
  ssh -o BatchMode=yes -o ConnectTimeout=5 "$HOST" "$COMMAND"
done
