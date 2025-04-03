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

logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

bf = BloomFilter(max_elements=1000000, error_rate=0.02, filename="/tmp/bloom.bin")
queue = Queue("/tmp/queue.bin", autosave=True)

def get_and_parse(url: str):

    try:
        response = requests.get(url)
    except:
        return None, None

    if response.status_code != 200:
        logging.info("Skipping status code: ", response.status_code)
        return None, None

    content_type = response.headers.get("Content-Type", "")
    if "text/html" not in content_type:
        logging.info("Skipping non-HTML content:", content_type)
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

index_path = "/tmp/index"
os.makedirs(index_path, exist_ok=True)

try:
    with open("/tmp/index_chunk", 'x') as f:
        f.write(str(0))
    logging.info("initialize index counter")
except Exception as _:
    logging.info("Crawler already init counter")

with open("init.txt") as f:
    for word in f.read().split():
        if word not in bf:
            logging.info("putting word : ", word)
            bf.add(word)
            queue.put(word)
        else:
            logging.info("skipping init word already in Bloom")


def get_chunk_number() -> int:
    with open("/tmp/index_chunk", 'r') as f:
        return int(f.read())

def write_chunk_number(num: int):
    with open("/tmp/index_chunk", 'w') as f:
        f.write(str(num))

pybind.alloc()

while not queue.empty() and not die:
    url = queue.get()
    queue.task_done()

    words, links = get_and_parse(url)
    if words is not None:

        for word in words:
            pybind.add_word(word)

        pybind.add_url(url)

        links = sorted(links, key=len)
        for link in links[:10]:
            if should_crawl(link) and queue.qsize() < 600:
                logging.info("LINK: ", link)
                queue.put(link)
                bf.add(link)

        if pybind.num_tokens() >= 500000:
            chunk_id = get_chunk_number()
            logging.info("Writing chunk number ", chunk_id)
            write_chunk_number(chunk_id + 1)
            pybind.write_blob(index_path + '/' + str(chunk_id))
            pybind.erase()
            pybind.alloc()

            logging.info("finish writing chunk")


bf.close()

if pybind.num_tokens() > 0:
    logging.info("Writing out hashmap with size : ", pybind.num_tokens())
    chunk_id = get_chunk_number()
    write_chunk_number(chunk_id + 1)
    pybind.write_blob(index_path + '/' + str(chunk_id))

pybind.erase()
logging.info("returning!")
