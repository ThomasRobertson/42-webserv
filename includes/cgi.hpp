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
		CgiHandler(Client client, Server server, std::string fileLocation, std::string cgiBinLocation, std::string body = "") :
		_client(client), _server(server), _fileLocation(fileLocation), _cgi_path(cgiBinLocation), _body(body)
		{
			_fIn = tmpfile();
			_fOut = tmpfile();
			if (_fIn == NULL || _fOut == NULL)
			{
				fclose(_fIn);
				fclose(_fOut);
				// std::cout << RED << "Could not create temp files for CGI.\n" << DEFAULT;
				throw std::runtime_error("");
			}
			_child_in_pipe = fileno(_fIn);
			_child_out_pipe = fileno(_fOut);
			if (_child_in_pipe == -1 || _child_out_pipe == -1)
			{
				fclose(_fIn);
				fclose(_fOut);
				// std::cout << RED << "Could not create temp files for CGI.\n" << DEFAULT;
				throw std::runtime_error("");
			}
		}
		~CgiHandler()
		{
			fclose(_fIn);
			fclose(_fOut);
		};
		std::string execute();

	private:
		Client _client;
		Server _server;

		std::string _fileLocation;
		std::string _cgi_path;
		std::vector<std::string> _environ;
		std::string _body;

		FILE *_fIn;
		FILE *_fOut;

		pid_t _child_pid;
		int _child_out_pipe;
		int _child_in_pipe;

		void build_args_env();
		void launch_child();
		void child_is_in_orbit();
		std::string capture_child_return();
		void sendBody();
		std::string generateReturnResponse(std::string return_str);
};
