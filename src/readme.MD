# Filestructure
- client: frontend server to deliver html to client browser and forward queries to query_server
- crawler: Crawler code including: requesting HTML, parsing HTML, parsing URLS, the frontier, robots.txt handling
- english: Mostly unused. includes: porter stemmer, word normalization (casing and removing non letter tokens), and URL prefix stripping
- index: in-memory hashtable, basic isrs (word and document isr as well as generic isr interface), serialization for hashblobs on disk
- lib: all the generic library code including: strings, vectors, lists, compression, math functions, among other data structures
- mock_crawl: deprecated. This was the original method for piping python crawler output to the c++ index before using PYBIND
- query: all code involved in handling client queries: ranking, constraint solver, language parsing, server to send and recv from the frontend
- test: Testing framework for CI/CD and ensuring we dont break things
