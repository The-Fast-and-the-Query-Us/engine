import requests
from bs4 import BeautifulSoup
import re
from urllib.parse import urljoin
from bloom_filter import BloomFilter
import queue
import subprocess
import time
import sys

def get_html(url: str) -> str | None:
    """Fetch HTML content of a URL."""
    try:
        response = requests.get(url, timeout=5)
        response.raise_for_status()  # Raise error if bad response
        return response.text
    except requests.RequestException as e:
        print(f"Error fetching {url}: {e}")
        return None

def extract_words_and_links(html: str, base_url: str):
    """Extract words and links from HTML."""
    soup = BeautifulSoup(html, "html.parser")

    # Extract visible text words (ignoring scripts and styles)
    for script in soup(["script", "style"]):
        script.decompose()  # Remove them

    text = soup.get_text(separator=" ")
    words = re.findall(r'\b\w+\b', text)  # Extract words

    # Extract all links
    links = {urljoin(base_url, a['href']) for a in soup.find_all("a", href=True)}

    return words, links


MAX_QUEUE = 500
q = queue.Queue()
bloom = BloomFilter(max_elements=1000000, error_rate=0.01)

start_urls = [
    "https://www.nytimes.com",
    "https://en.wikipedia.org/wiki/C%2B%2B"
]

# initialize crawl
for url in start_urls:
    bloom.add(url)
    q.put(url)

# start c++ client
process = subprocess.Popen(["./mock", "./index"], stdin=subprocess.PIPE, stderr=sys.stdout, stdout=sys.stdout, text=True)

crawl_count = 0
start = time.time()
# run crawl
while q.qsize() > 0 and time.time() - start < 30:
    url = q.get()
    html = get_html(url)
    if html is not None:
        words, links = extract_words_and_links(html, url)
        crawl_count += 1

        try:
            for word in words:
                process.stdin.write("1\n")
                process.stdin.write(word+'\n')
            process.stdin.write("0\n")
            process.stdin.write(url+'\n')

        except Exception as e:
            print(f"error {e}")
            print("getting c++ status")
            return_val = process.poll()
            if return_val is None:
                process.terminate()
                process.wait()
                print("not done")
            else:
                print(f"return {return_val}")
            exit(1)

        for link in links:
            if q.qsize() == MAX_QUEUE:
                break
            if link not in bloom:
                bloom.add(link)
                q.put(link)

print("killing process")
process.stdin.write("q\n") #quit program
process.wait()

print("blobber returned status", process.returncode)
print(f"crawled {crawl_count} sites")
