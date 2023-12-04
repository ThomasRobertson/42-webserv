#pragma once

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>
#include <map>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "Settings.hpp"
#include "StartServers.hpp"
#include "utils.hpp"
#include "ConfigFile.hpp"
#include "ClientResponse.hpp"
#include "GenerateMethod.hpp"

#define PIPE_READ 0
#define PIPE_WRITE 1

#define CGI_BUFFER_SIZE 1024
#define GATEWAY_INTERFACE "CGI/1.1" //?Can we change that ?


class CgiHandler
{
	public:
		CgiHandler(Client client, Server server, std::string fileLocation, std::string cgiBinLocation, std::string body = "") : _client(client), _server(server), _fileLocation(fileLocation), _cgi_path(cgiBinLocation), _body(body) {}
		~CgiHandler() {};
		std::string execute();

	private:
		Client _client;
		Server _server;

		std::string _fileLocation;
		std::string _cgi_path;
		std::vector<std::string> _environ;
		std::string _body;

		pid_t _child_pid;
		int _child_out_pipe[2];
		int _child_in_pipe[2];

		void build_args_env();
		void launch_child();
		void child_is_in_orbit();
		std::string capture_child_return();
		void sendBody();
		std::string generateReturnResponse(std::string return_str);
};

