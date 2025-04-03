import signal
from urllib.parse import urljoin, urlparse
import re
import sys
import os
import requests
import logging
from bs4 import BeautifulSoup
from bloom_filter2 import BloomFilter
from persistqueue import Queue

build_dir = os.path.abspath("./build")
sys.path.append(build_dir) # to include non-working directory library

import pybind
# PYBIND INTERFACE (LSP sucks)
#m.def("alloc", &alloc, "Allocate new hashtable");
#m.def("erase", &erase, "delete hashtable");
#m.def("add_word", &add_word);
#m.def("add_url", &add_url);
#m.def("num_tokens", &get_word_count);
#m.def("write_blob", &write_blob);

IGNORED_EXTENSIONS = {".jpg", ".jpeg", ".png", ".gif", ".pdf", ".zip", ".mp4", ".mp3"}

logger = logging.getLogger()
bf = BloomFilter(max_elements=1000000, error_rate=0.02, filename="/tmp/bloom.bin")
queue = Queue("/tmp/queue.bin", autosave=True)

def get_and_parse(url: str):
    response = requests.get(url)

    if response.status_code != 200:
        logger.info("Skipping status code: ", response.status_code)
        return None, None

    content_type = response.headers.get("Content-Type", "")
    if "text/html" not in content_type:
        logger.info("Skipping non-HTML content:", content_type)
        return None, None

    soup = BeautifulSoup(response.text, "html.parser")

    for script in soup(["script", "style"]):
        script.extract()

    text = soup.get_text()
    text = re.sub(r'\s+', ' ', text)

    words = text.lower().split()
    links = {urljoin(url, a['href']) for a in soup.find_all('a', href=True)}
    return words, links

def should_crawl(url: str) -> bool:
    if url in bf:
        return False

    parsed = urlparse(url)

    if any(parsed.path.lower().endswith(ext) for ext in IGNORED_EXTENSIONS):
        return False

    if any(x in parsed.path.lower() for x in ["/logout", "/login", "/signup", "/admin"]):
        return False

    return True

die = False

def sig_int_handle(signal, frame):
    print("sigint recved setting flag")
    global die
    die = True

signal.signal(signal.SIGINT, sig_int_handle)

while not queue.empty() and not die:
    url = queue.get()
    queue.task_done()

    words, links = get_and_parse(url)
    if words is not None:

        for word in words:
            pybind.add_word(word)

        pybind.add_url(url)


