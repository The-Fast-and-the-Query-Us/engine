#!/bin/bash

HOSTS=(
  engine1 engine2 engine3
  sui1 sui2 sui3
  rose1 rose2 rose3
  jimmy1 jimmy2 jimmy3
  zur1 zur2 zur3
  lander1 lander2 lander3
)

tmpfile=$(mktemp)

get_last_2() {
  local host=$1
  output=$(ssh "$host" "tail -n 2 ~/.local/share/crawler/docs.log" 2>/dev/null || echo -e "0\n0")
  start=$(echo "$output" | sed -n 1p)
  end=$(echo "$output" | sed -n 2p)
  start=${start:-0}
  end=${end:-0}
  echo "$start $end $host" >> "$tmpfile"
}

for host in "${HOSTS[@]}"; do
  get_last_2 "$host" &
done

wait

TOTAL_RATE=0
now_s=$(date +%s)
now_ns=$(date +%N)
now_ms=$(( now_s * 1000 + now_ns / 1000000 ))

while read -r start end host; do
  DIFF=$(( end - start ))
  if [ "$DIFF" -gt 0 ]; then
    RATE=$(( 4096 * 1000 / DIFF ))
    DELTA=$(((now_ms - end) / 1000))

    if [ "$DELTA" -gt 60 ]; then
      echo "WARNING: $host no recent updates"
    fi

    echo "$RATE docs per second on $host"
    TOTAL_RATE=$((TOTAL_RATE + RATE))
  else
    echo "Invalid time difference on $host (start=$start, end=$end)"
  fi
done < "$tmpfile"

echo "Total Rate: $TOTAL_RATE docs per second"

# Clean up
rm "$tmpfile"
