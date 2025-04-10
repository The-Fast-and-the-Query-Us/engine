import os

# Load allowed URLs from urls.txt into a set for fast lookup
with open("crawled_urls.txt", "r", encoding="utf-8") as f:
    valid_urls = set(line.strip() for line in f if line.strip())

# Prepare directories
input_dir = "results-parsed"
output_dir = "results-cleaned"
os.makedirs(output_dir, exist_ok=True)

# Process each .txt file
for filename in os.listdir(input_dir):
    if filename.endswith(".txt"):
        input_path = os.path.join(input_dir, filename)
        output_path = os.path.join(output_dir, filename)

        with open(input_path, "r", encoding="utf-8") as infile:
            urls = [line.strip() for line in infile if line.strip()]

        # Keep only URLs that are in urls.txt
        cleaned_urls = [url for url in urls if url in valid_urls]

        with open(output_path, "w", encoding="utf-8") as outfile:
            outfile.write("\n".join(cleaned_urls) + "\n")
