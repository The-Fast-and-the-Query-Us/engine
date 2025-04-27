# The Fast and The Query Us - Search Engine
Archived code for our search engine
## File structure
- src: our production code for running the search engine
- mock_crawl: python bindings that were used to test the index when the crawler wasn't finished (now depricated)
- training: scrapped results from duck duck go that we used to attempt gradient decent tuning of the ranker
- systemd: systemd configs to allow us to run the crawler and query_server with nice_value=-20 and automatic restart
- connectivity: two small test scripts to ensure that we have correct firewall rules to send and receive packets on GCP
- frontend: html, images, and js to serve for our frontend server.
- ips.txt: list of server ips so that servers can contact each other for distributing the workload
- We also have an assortment of shell scripts for sshing into GCP we would like to move to a directory but havent done so yet to avoid breaking relative paths
