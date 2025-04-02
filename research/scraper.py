import os
import json
import time
import click
import requests
import bs4
from tqdm import tqdm

DUCKDUCKGO_URL = "https://html.duckduckgo.com/html/"
RESULTS_DIR = "results"


def search(query: str) -> list[str]:
    """Perform a search on DuckDuckGo and return a list of result URLs."""
    data = {"q": query, "kl": "en-us"}
    try:
        response = requests.post(DUCKDUCKGO_URL, data=data, timeout=10)
        response.raise_for_status()
    except requests.RequestException as e:
        click.echo(f"Error fetching results for '{query}': {e}")
        return []

    soup = bs4.BeautifulSoup(response.text, "html.parser")
    results = soup.find_all("a", class_="result__url")
    return [
        str(result.get("href"))
        for result in results
        if isinstance(result, bs4.element.Tag) and result.get("href")
    ]


def save_results(query: str, results: list[str]):
    """Save search results to a JSON file."""
    assert len(results) == 10 and isinstance(results[0], str)
    os.makedirs(RESULTS_DIR, exist_ok=True)
    filename = os.path.join(RESULTS_DIR, f"{query}.json")
    with open(filename, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2)


@click.command()
@click.option(
    "-file",
    "file_path",
    required=True,
    type=click.Path(exists=True),
    help="Path to the input file containing queries (one per line).",
)
def main(file_path):
    """Scrape DuckDuckGo search results sequentially with a delay, using a progress bar."""
    with open(file_path, "r", encoding="utf-8") as f:
        queries = [line.strip() for line in f if line.strip()]

    with tqdm(
        total=len(queries), desc="Scraping Queries", unit="query"
    ) as pbar:
        for query in queries:
            filename = os.path.join(RESULTS_DIR, f"{query}.json")

            if os.path.exists(filename):
                pbar.update(1)
                continue

            results = search(query)
            save_results(query, results)

            time.sleep(1)

            pbar.update(1)


if __name__ == "__main__":
    main()
