import requests
from bs4 import BeautifulSoup
import re

def get_html(url):
    """Fetch HTML content of a URL."""
    try:
        response = requests.get(url, timeout=5)
        response.raise_for_status()  # Raise error if bad response
        return response.text
    except requests.RequestException as e:
        print(f"Error fetching {url}: {e}")
        return None

def extract_words_and_links(html):
    """Extract words and links from HTML."""
    soup = BeautifulSoup(html, "html.parser")

    # Extract visible text words (ignoring scripts and styles)
    for script in soup(["script", "style"]):
        script.decompose()  # Remove them

    text = soup.get_text(separator=" ")
    words = re.findall(r'\b\w+\b', text)  # Extract words

    # Extract all links
    links = {a['href'] for a in soup.find_all("a", href=True)}

    return words, links

def crawl_urls(urls):
    """Process a list of URLs."""
    for url in urls:
        print(f"Fetching {url}...")
        html = get_html(url)
        if html:
            words, links = extract_words_and_links(html)
            print(f"Found {len(words)} words and {len(links)} links in {url}")
            print("Sample Links:", list(links)[:5])  # Show first 5 links

# List of URLs to start with
start_urls = [
    "https://amazon.com",
    "https://www.python.org"
]

crawl_urls(start_urls)
