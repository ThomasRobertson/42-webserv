#pragma once

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>
#include <map>

#include "Settings.hpp"
#include "StartServers.hpp"
#include "utils.hpp"
#include "ConfigFile.hpp"

#define PIPE_READ 0
#define PIPE_WRITE 1

#define CGI_BUFFER_SIZE 1024
#define GATEWAY_INTERFACE "CGI/1.1" //?Can we change that ?


class CgiHandler
{
	public:
		CgiHandler(Client client, Server server, std::string fileLocation, std::string cgiBinLocation) : _client(client), _server(server), _fileLocation(fileLocation), _cgi_path(cgiBinLocation) {}
		~CgiHandler() {};
		std::string execute();

	private:
		Client _client;
		Server _server;
		std::string _fileLocation;

		std::string _cgi_path;
		std::vector<std::string> _args;
		std::map<std::string, std::string> _environ;

		pid_t _child_pid;
		int _child_pipe[2];

		void build_args_env();
		void launch_child();
		void child_is_in_orbit();
		std::string capture_child_return();
		// void check_args_env() const; //?Needed ?


};

