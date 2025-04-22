#!/bin/bash

COMMAND="cd engine && git restore . && git fetch && git checkout ranker_final && git pull && ./frontend_init.sh"

ssh -o ConnectTimeout=5 engine1 "$COMMAND"

echo "Frontend started!"
head -n 1 ips.txt
