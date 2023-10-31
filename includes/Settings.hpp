#pragma once

#define DEBUG true

#ifdef DEBUG
# include <cassert>
#endif // DEBUG

#define SERVER_SOFTWARE "Webserv/42.1"
#define SERVER_PROTOCOL "HTTP/1.1"
#define AUTH_TYPE "Basic"

#define HTML_DIR "www" //where the html files are located

#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define YELLOW  "\x1b[33m"
#define DEFAULT "\x1b[0m"