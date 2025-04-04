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
import string

build_dir = os.path.abspath("./build")
sys.path.append(build_dir)  # to include non-working directory library

import pybind

# PYBIND INTERFACE (LSP sucks)
# m.def("alloc", &alloc, "Allocate new hashtable");
# m.def("erase", &erase, "delete hashtable");
# m.def("add_word", &add_word);
# m.def("add_url", &add_url);
# m.def("num_tokens", &get_word_count);
# m.def("write_blob", &write_blob);

IGNORED_EXTENSIONS = {
    ".jpg",
    ".jpeg",
    ".png",
    ".gif",
    ".pdf",
    ".zip",
    ".mp4",
    ".mp3",
}

logging.basicConfig(
    level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s"
)

BASE_PATH = "~/.local/share/crawler"

os.makedirs(BASE_PATH, exist_ok=True)

def good_domain_authority(url: str) -> bool:
    """Simulate domain authority based on domain type."""
    domain = urlparse(url).netloc
    if domain.endswith(".gov") or domain.endswith(".edu"):
        return True
    elif domain.endswith(".org"):
        return True
    elif domain.endswith(".com"):
        return True
    else:
        return False


def url_depth(url: str) -> int:
    """Calculate depth based on the number of path segments."""
    path = urlparse(url).path.strip("/")
    return len(path.split("/")) if path else 0


def get_and_parse(url: str):
    try:
        response = requests.get(url, timeout=3)
    except:
        return None, None

    if response.status_code != 200:
        logging.info("Skipping status code: %d", response.status_code)
        return None, None

    content_type = response.headers.get("Content-Type", "")
    if "text/html" not in content_type:
        logging.info("Skipping non-HTML content: %s", content_type)
        return None, None

    soup = BeautifulSoup(response.text, "html.parser")

    html_tag = soup.find("html")
    if html_tag and html_tag.has_attr("lang"):
        if html_tag["lang"] != "en":
            logging.info("skip non english page")
            return None, None

    for script in soup(["script", "style"]):
        script.extract()

    text = soup.get_text(separator=" ", strip=True)

    # Remove punctuation efficiently
    text = text.translate(str.maketrans("", "", string.punctuation))

    # Convert to lowercase and split into words
    words = text.lower().split()

    links = {urljoin(url, a["href"]) for a in soup.find_all("a", href=True)}
    return words, links


def should_crawl(url: str) -> bool:
    parsed = urlparse(url)

    if any(parsed.path.lower().endswith(ext) for ext in IGNORED_EXTENSIONS):
        return False

    if any(
        x in parsed.path.lower()
        for x in ["/logout", "/login", "/signup", "/admin"]
    ):
        return False

    if not good_domain_authority(url):
        return False

    if url_depth(url) >= 5:
        return False

    BLOCKED_DOMAINS = {"api.", "cdn.", "drive.google.com"}

    if any(blocked in parsed.netloc.lower() for blocked in BLOCKED_DOMAINS):
        return False

    return True

CHUNK_NUM_PATH = BASE_PATH + "/index_chunk"

INDEX_PATH = BASE_PATH + "/index"

def maybe_init_chunk():
    try:
        with open(CHUNK_NUM_PATH, "x") as f:
            f.write(str(0))
    except:
        logging.info("Skipping chunk init")

def get_chunk_number() -> int:
    with open(CHUNK_NUM_PATH, "r") as f:
        return int(f.read())

def write_chunk_number(num: int):
    with open(CHUNK_NUM_PATH, "w") as f:
        f.write(str(num))


def same_domain(url1, url2):
    domain1 = urlparse(url1).netloc.lower()
    domain2 = urlparse(url2).netloc.lower()
    return domain1 == domain2

def main():
    bf = BloomFilter(max_elements=1000000, error_rate=0.01, filename=BASE_PATH + "/bloom.bin")
    queue = Queue(BASE_PATH + "/queue.bin", autosave=True)

    os.makedirs(INDEX_PATH, exist_ok=True)

    die = False
    def sig_int_handle(signal, frame):
        print("sigint recved setting flag")
        nonlocal die
        die = True
    signal.signal(signal.SIGINT, sig_int_handle)

    maybe_init_chunk()

    with open("init.txt", "r") as seeds:
        for seed in seeds:
            stripped = seed.strip()
            if stripped not in bf:
                queue.put(stripped)
                bf.add(stripped)

    pybind.alloc()

    while not die and not queue.empty():
        url = queue.get()
        queue.task_done()

        words, links = get_and_parse(url)
        if words is not None and links is not None:
            for word in words:
                pybind.add_word(word)

            pybind.add_url(url)

            if pybind.num_tokens() > 1000000:
                chunk_id = get_chunk_number()
                pybind.write_blob(INDEX_PATH + '/' + str(chunk_id))
                write_chunk_number(chunk_id + 1)

            link_count = 0
            same_count = 0
            for link in links:
                if queue.qsize() > 2000 or link_count > 7:
                    break

                if link in bf:
                    continue

                if should_crawl(link):
                    if same_domain(link, url) and same_count > 2:
                        continue
                    if same_domain(link, url):
                        same_count += 1
                    link_count += 1
                    bf.add(link)
                    queue.put(link)

    if pybind.num_tokens() > 0:
        chunk_id = get_chunk_number()
        pybind.write_blob(INDEX_PATH + '/' + str(chunk_id))
        write_chunk_number(chunk_id + 1)

    pybind.erase()


if __name__ == '__main__':
    main()

