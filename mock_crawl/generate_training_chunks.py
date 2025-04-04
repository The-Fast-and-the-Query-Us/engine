import types
import bs4
import click
import logging
import os
from pathlib import Path
import signal
import string
import sys
import asyncio
import aiohttp

build_dir = os.path.abspath("./build")
sys.path.append(build_dir)  # to include non-working directory library

import pybind  # pyright: ignore reportMissingImports

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%m/%d/%Y %H:%M:%S",
    handlers=[logging.StreamHandler(sys.stdout)],
)

BASE_PATH = Path.home() / ".local" / "share" / "crawler" / "training"
INDEX_PATH = BASE_PATH / "index"
CHUNK_NUM_PATH = BASE_PATH / "index_chunk"

die = False

HEADERS = {
    "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:138.0) Gecko/20100101 Firefox/138.0",
}


def sig_int_handle(signum: int, frame: types.FrameType | None):
    logging.critical(f"Received signal {signum} from frame {frame}")
    global die
    die = True


signal.signal(signal.SIGINT, sig_int_handle)


def initialize_chunk_count():
    try:
        with open(CHUNK_NUM_PATH, "x") as chunk_file:
            chunk_file.write(str(0))
        logging.info(f"Initialized {CHUNK_NUM_PATH} to 0")
    except FileExistsError as _:
        with open(CHUNK_NUM_PATH, "r") as chunk_file:
            logging.info(
                f"{CHUNK_NUM_PATH} already initialized to "
                f"{chunk_file.readline().strip()}"
            )


def get_chunk_number() -> int:
    with open(CHUNK_NUM_PATH, "r") as f:
        return int(f.read())


def write_chunk_number(num: int):
    with open(CHUNK_NUM_PATH, "w") as f:
        f.write(str(num))


async def get_words(
    url: str, session: aiohttp.ClientSession
) -> tuple[str, list[str]]:
    try:
        async with session.get(
            url, headers=HEADERS, timeout=aiohttp.ClientTimeout(total=10)
        ) as response:
            if response.status != 200:
                logging.info(
                    "Skipping %s, status code: %d", url, response.status
                )
                return url, []

            content_type = response.headers.get("Content-Type", "")
            if "text/html" not in content_type:
                logging.info(
                    "Skipping %s, non-HTML content: %s", url, content_type
                )
                return url, []

            html = await response.text()
    except Exception as e:
        logging.warning("Failed to fetch %s: %s", url, str(e))
        return url, []

    soup = bs4.BeautifulSoup(html, "html.parser")

    html_tag = soup.find("html")
    if isinstance(html_tag, bs4.element.Tag) and html_tag.has_attr("lang"):
        lang = str(html_tag["lang"]).lower().strip()
        if not lang.startswith("en"):
            logging.info("Skipping %s, non-English page: lang=%s", url, lang)
            return url, []

    for script in soup(["script", "style"]):
        script.extract()

    text = soup.get_text(separator=" ", strip=True)
    text = text.translate(str.maketrans("", "", string.punctuation))
    words = text.lower().split()

    return url, words


def write_chunk():
    chunk_id = get_chunk_number()
    logging.info("Writing chunk number %d", chunk_id)
    write_chunk_number(chunk_id + 1)
    pybind.write_blob(str(INDEX_PATH / str(chunk_id)))
    pybind.erase()
    pybind.alloc()

    logging.info("finish writing chunk")


def add_to_index(url: str, words: list[str]):
    if not words:
        return

    for word in words:
        pybind.add_word(word)
    pybind.add_url(url)
    print(url)

    if pybind.num_tokens() >= 500000:
        write_chunk()


async def process_urls(urls: list[str]):
    url_to_words: dict[str, list[str]] = {}

    async with aiohttp.ClientSession() as session:
        tasks = [get_words(url, session) for url in urls]
        for future in asyncio.as_completed(tasks):
            if die:
                break
            url, words = await future
            url_to_words[url] = words

    return url_to_words


@click.command()
@click.argument("filename", type=click.Path(exists=True, readable=True))
def main(filename):
    pybind.alloc()

    initialize_chunk_count()

    with open(filename, "r", encoding="utf-8") as file:
        urls = [line.strip() for line in file if line.strip()]

    url_to_words = asyncio.run(process_urls(urls))

    for url, words in url_to_words.items():
        add_to_index(url, words)

    if pybind.num_tokens() > 0:
        logging.info("Writing out hashmap with size : %d", pybind.num_tokens())
        chunk_id = get_chunk_number()
        write_chunk_number(chunk_id + 1)
        pybind.write_blob(str(INDEX_PATH / str(chunk_id)))
    pybind.erase()


if __name__ == "__main__":
    main()
