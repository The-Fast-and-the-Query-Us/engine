import os
import json
import re

input_dir = "results"
output_dir = "results-parsed"

# Ensure the output directory exists
os.makedirs(output_dir, exist_ok=True)

# Process each JSON file in the input directory
for filename in os.listdir(input_dir):
    if filename.endswith(".json"):
        query_name = filename[:-5]  # strip ".json"
        input_path = os.path.join(input_dir, filename)

        clean_query = re.sub(r"[^a-zA-Z0-9 ]", "", query_name)
        words = clean_query.split()
        custom_query = ""
        for i, word in enumerate(words):
            if i == len(words) - 1:
                custom_query += word
            else:
                custom_query += word + " + "
        output_path = os.path.join(output_dir, f"{custom_query}.txt")
        with open(input_path, "r", encoding="utf-8") as infile:
            urls = json.load(infile)

        with open(output_path, "w", encoding="utf-8") as outfile:
            outfile.write("\n".join(urls) + "\n")
