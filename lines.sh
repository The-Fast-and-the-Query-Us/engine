#!/bin/bash

# Set folder to current directory if none provided
FOLDER="${1:-.}"

# Check if folder exists
if [ ! -d "$FOLDER" ]; then
  echo "Error: Directory '$FOLDER' does not exist."
  exit 1
fi

# Navigate to the folder
cd "$FOLDER" || exit 1

# Check if we're in a git repository
if ! git rev-parse --is-inside-work-tree > /dev/null 2>&1; then
  echo "Error: Not a git repository or git command failed."
  exit 1
fi

# File extensions to include (C++ files)
FILE_EXTENSIONS="\.cpp$|\.hpp$"

echo "Analyzing C++ files (*.cpp, *.hpp) in $(pwd)..."
echo "-----------------------------------------------------------"

# Calculate and store total lines first (filtered for C++ files)
TOTAL_LINES=$(git ls-files | grep -E "$FILE_EXTENSIONS" | xargs -n1 git blame --line-porcelain 2>/dev/null | sed -n 's/^author //p' | wc -l)
echo "Total lines of C++ code: $TOTAL_LINES"
echo "-----------------------------------------------------------"
echo "Lines by author (percentage):"
echo "-----------------------------------------------------------"

# Use the compact command to get author stats and calculate percentages (filtered for C++ files)
git ls-files | grep -E "$FILE_EXTENSIONS" | xargs -n1 git blame --line-porcelain 2>/dev/null | sed -n 's/^author //p' | sort -f | uniq -ic | sort -nr | 
while read -r COUNT AUTHOR; do
  PERCENTAGE=$(awk "BEGIN {printf \"%.2f\", ($COUNT/$TOTAL_LINES)*100}")
  printf "%-30s %8d lines (%6.2f%%)\n" "$AUTHOR" "$COUNT" "$PERCENTAGE"
done

echo "-----------------------------------------------------------"
exit 0
