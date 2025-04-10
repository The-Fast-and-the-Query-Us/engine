import os

directory = "results-cleaned"  # replace with your target directory

for filename in os.listdir(directory):
    if "+" in filename:
        new_filename = filename.replace("+", "|")
        src = os.path.join(directory, filename)
        dst = os.path.join(directory, new_filename)
        os.rename(src, dst)
