#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <netdb.h>
#include <cstring>

class Client
{
    private:
		sockaddr_in *_address;
    public:
        Client(sockaddr *address);
        ~Client();

		void printIpBytes();
		std::string getIpAddress();
		bool operator==(const Client &toCompare);
		// bool operator == (const Car &c)
};

#endif
