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


die = False


def sig_int_handle(signal, frame):
    print("sigint recved setting flag")
    global die
    die = True


signal.signal(signal.SIGINT, sig_int_handle)

index_path = "/tmp/training-index"
os.makedirs(index_path, exist_ok=True)

try:
    with open("/tmp/training_chunk", "x") as f:
        f.write(str(0))
    logging.info("initialize index counter")
except Exception as _:
    logging.info("Crawler already init counter")

queue: list[str] = []

with open("training_urls.txt") as f:
    for url in f.read().split():
        logging.info("putting word : %s", url)
        queue.append(url)


def get_chunk_number() -> int:
    with open("/tmp/index_chunk", "r") as f:
        return int(f.read())


def write_chunk_number(num: int):
    with open("/tmp/index_chunk", "w") as f:
        f.write(str(num))


while queue and not die:
    url = queue.pop()

    words, links = get_and_parse(url)

    if words is not None:
        for word in words:
            pybind.add_word(words)

        pybind.add_url(url)

        if pybind.num_tokens() >= 500000:
            chunk_id = get_chunk_number()
            logging.info("Writing chunk number %d", chunk_id)
            write_chunk_number(chunk_id + 1)
            pybind.write_blob(index_path + "/" + str(chunk_id))
            pybind.erase()
            pybind.alloc()

            logging.info("finish writing chunk")


# exiting
if pybind.num_tokens() > 0:
    logging.info("Writing out hashmap with size : %d", pybind.num_tokens())
    chunk_id = get_chunk_number()
    write_chunk_number(chunk_id + 1)
    pybind.write_blob(index_path + "/" + str(chunk_id))

pybind.erase()
logging.info("returning!")
