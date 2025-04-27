#!/bin/bash

HOSTS=(
  engine1 engine2 engine3
  sui1 sui2 sui3
  rose1 rose2 rose3
  jimmy1 jimmy2 jimmy3
  zur1 zur2 zur3
  lander1 lander2 lander3
)

# Create a temp file to store line counts
tmpfile=$(mktemp)

# Function to get line count on a host
get_line_count() {
  local host=$1
  line_count=$(ssh "$host" "wc -l ~/.local/share/crawler/docs.log | awk '{print \$1}'" 2>/dev/null || echo 0)

  if [ "$line_count" -gt 2850 ]; then
    ssh "$host" "sudo systemctl stop crawler.service"
    echo "STOPPED: $host"
  fi

  echo "$line_count $host" >> "$tmpfile"
}

# Launch all SSH in background
for host in "${HOSTS[@]}"; do
  get_line_count "$host" &
done

# Wait for all background jobs to finish
wait

# Sum up results
total_lines=0
while read -r line host; do
  echo "$line on $host"
  total_lines=$((total_lines + line))
done < "$tmpfile"

# Clean up
rm "$tmpfile"

# Final result
result=$((total_lines * 4096))
echo "Total docs: $result"

