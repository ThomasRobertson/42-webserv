// #include "Client.hpp"
// #include <arpa/inet.h>

// Client::Client(sockaddr *address)
// {
// 	_address = reinterpret_cast<struct sockaddr_in*>(address);
// }

// Client::~Client()
// {

// }

// bool Client::operator==(const Client &toCompare)
// {
// 	// std::cout << "COMPARING..." << std::endl;
// 	if (this->_address->sin_addr.s_addr == toCompare._address->sin_addr.s_addr)
// 		return true;
// 	return false;
// }

// void Client::printIpBytes()
// {
// 	// char hexChars[] = "0123456789ABCDEF";

// 	// std::cout << "IP BYTES: ";
// 	// for (int i = 0; i < sizeof(this->_address.sa_data); i++)
// 	// {
//     //     char byte = this->_address.sa_data[i];
//     //     char hex[4];
//     //     hex[0] = hexChars[(byte >> 4) & 0xF];
//     //     hex[1] = hexChars[byte & 0xF];
//     //     hex[2] = ' ';
//     //     hex[3] = 0;
        
//     //     std::cout << hex;
//     // }
// 	// std::cout << std::endl;
// }

// std::string Client::getIpAddress()
// {
// 	char *s = inet_ntoa(this->_address->sin_addr);
// 	return s;
// }