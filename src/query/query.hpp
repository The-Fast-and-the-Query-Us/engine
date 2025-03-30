#pragma once

namespace fast::query {

/*
* Handles a clients request, leaving fd open
*/
void handle(const int client_fd, const int num_chunks);

}
