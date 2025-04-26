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

# Initialize total line count
total_lines=0

# Loop over each machine
for host in "${HOSTS[@]}"; do
  # Run the wc -l command over SSH and get the line count
  line_count=$(ssh "$host" "wc -l ~/.local/share/crawler/docs.log | awk '{print \$1}'")
  
  # Add the line count to the total
  total_lines=$((total_lines + line_count))
done

# Multiply the total line count by 4096 and print the result
result=$((total_lines * 4096))
echo "Total count times 4096: $result"
