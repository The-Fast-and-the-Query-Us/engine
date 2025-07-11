import os
import requests
from bs4 import BeautifulSoup
import re
from urllib.parse import urljoin
from bloom_filter import BloomFilter
import queue
import time
import subprocess
import sys
import atexit

def get_html(url: str) -> str | None:
    """Fetch HTML content of a URL."""
    try:
        response = requests.get(url, timeout=5)
        response.raise_for_status()  # Raise error if bad response
        return response.text
    except requests.RequestException as e:
        print(e)
        return None

def extract_words_and_links(html: str, base_url: str):
    """Extract words and links from HTML."""
    soup = BeautifulSoup(html, "html.parser")

    # Extract visible text words (ignoring scripts and styles)
    for script in soup(["script", "style"]):
        script.decompose()  # Remove them

    text = soup.get_text(separator=" ").lower()
    text = re.sub(r'[^\w\s]', '', text)

    # Extract all links
    links = {urljoin(base_url, a['href']) for a in soup.find_all("a", href=True)}

    return text.split(), list(links)


MINUTE = 60
TIME_LIMIT = 10 * MINUTE

MAX_QUEUE = 500
q = queue.Queue()
bloom = BloomFilter(max_elements=1000000, error_rate=0.01)

start_urls = [
    "https://www.imdb.com/title/tt1013752/?ref_=nv_sr_srsg_0_tt_7_nm_1_in_0_q_fast%2520a",
    "https://www.nytimes.com",
    "https://en.wikipedia.org/wiki/C%2B%2B"
]

# initialize crawl
for url in start_urls:
    bloom.add(url)
    q.put(url)

# init c++ client
index_path = os.path.abspath("./index")
blobber_path = "../src/build/mock_crawl/mock"
process = subprocess.Popen([blobber_path, index_path], stdin=subprocess.PIPE, stdout=sys.stdout, stderr=sys.stderr, text=True)

def cleanup():
    try:
        process.stdin.close()
    except Exception as e:
        print(e)

    try:
        result = process.wait(timeout=3)
        print(f"blobber exited with code : {result}")
    except:
        process.terminate()
        print("terminating process")

atexit.register(cleanup)

start = time.time()

# run crawl
while q.qsize() > 0 and time.time() - start < TIME_LIMIT:
    url = q.get()
    html = get_html(url)
    if html is not None:
        words, links = extract_words_and_links(html, url)

        for word in words:
            process.stdin.write("1\n")
            process.stdin.write(word + '\n')
        process.stdin.write("0\n")
        process.stdin.write(url + '\n')

        for link in links[:8]:
            if q.qsize() == MAX_QUEUE:
                break
            if link not in bloom:
                bloom.add(link)
                q.put(link)
