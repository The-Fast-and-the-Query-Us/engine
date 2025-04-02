import random
from faker import Faker

fake = Faker()
NUM_QUERIES = 1000


def generate_random_query() -> str:
    """Generate a random search query."""
    return fake.sentence(nb_words=random.randint(1, 6)).strip()[:-1].lower()


def generate_queries(num_queries: int) -> list[str]:
    """Generate a list of random search queries."""
    return [generate_random_query() for _ in range(num_queries)]


def write_queries_to_file(filename: str, queries: list[str]):
    """Write the list of search queries to a text file."""
    with open(filename, "w", encoding="utf-8") as f:
        for query in queries:
            f.write(query + "\n")


if __name__ == "__main__":
    queries = generate_queries(NUM_QUERIES)
    write_queries_to_file("input.txt", queries)
    print(
        f"Successfully generated {NUM_QUERIES} random search queries in 'input.txt'."
    )
