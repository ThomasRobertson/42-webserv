#pragma once

#define DEBUG true

#ifdef DEBUG
# include <cassert>
#endif // DEBUG

#define SERVER_SOFTWARE "Webserv/42.1"
#define SERVER_PROTOCOL "HTTP/1.1"
#define AUTH_TYPE "Basic"

#define HTML_DIR "www" //where the html files are located