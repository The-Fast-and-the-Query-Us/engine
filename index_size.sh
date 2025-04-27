#!/bin/bash

# List of machines (replace with your actual machine names or IPs)
machines=(
  engine1 engine2 engine3
  sui1 sui2 sui3
  rose1 rose2 rose3
  jimmy1 jimmy2 jimmy3
  zur1 zur2 zur3
  lander1 lander2 lander3
)

# Initialize variables
total_size=0
max_size=0

# Function to convert size to bytes
convert_to_bytes() {
  local size=$1
  local unit=$2

  case $unit in
    K) echo $((size * 1024));;
    M) echo $((size * 1024 * 1024));;
    G) echo $((size * 1024 * 1024 * 1024));;
    T) echo $((size * 1024 * 1024 * 1024 * 1024));;
    P) echo $((size * 1024 * 1024 * 1024 * 1024 * 1024));;
    *) echo $size;;  # Assuming it's already in bytes
  esac
}

# Loop through each machine
for machine in "${machines[@]}"; do
  # Run SSH command to get the size of the directory and handle errors
  size=$(ssh -o ConnectTimeout=5 "$machine" "du -sh ~/.local/share/crawler/index | cut -f1" 2>/dev/null)

  # If SSH command fails, skip this machine
  if [ -z "$size" ]; then
    echo "Error: Unable to connect to $machine."
    continue
  fi

  # Extract size value and unit
  size_value=$(echo $size | grep -oE '[0-9]+')
  unit=$(echo $size | grep -oE '[A-Za-z]+')

  # Convert size to bytes
  size_bytes=$(convert_to_bytes $size_value $unit)

  # Add to total size
  total_size=$((total_size + size_bytes))

  # Check if this is the maximum size
  if [ "$size_bytes" -gt "$max_size" ]; then
    max_size=$size_bytes
  fi
done

# Convert total size back to human-readable format manually
total_size_human=$(echo $total_size | awk '{if($1 >= 1024*1024*1024) {print $1/(1024*1024*1024) " GB"} else if($1 >= 1024*1024) {print $1/(1024*1024) " MB"} else if($1 >= 1024) {print $1/1024 " KB"} else {print $1 " bytes"}}')

# Convert max size to human-readable format manually
max_size_human=$(echo $max_size | awk '{if($1 >= 1024*1024*1024) {print $1/(1024*1024*1024) " GB"} else if($1 >= 1024*1024) {print $1/(1024*1024) " MB"} else if($1 >= 1024) {print $1/1024 " KB"} else {print $1 " bytes"}}')

echo "Total size: $total_size_human"
echo "Max size: $max_size_human"

