#pragma once

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>
#include <map>

#include "utils.hpp"
#include "ConfigFile.hpp"

#define PIPE_READ 0
#define PIPE_WRITE 1

#define CGI_BUFFER_SIZE 1024
#define GATEWAY_INTERFACE "CGI/1.1" //?Can we change that ?
#define SERVER_SOFTWARE "Webserv/42.1"
#define SERVER_PROTOCOL "HTTP/1.1"
#define AUTH_TYPE "Basic"

class CgiHandler
{
	public:
		struct cgi_env
		{
			int i;
		};
		

		CgiHandler(std::string file_path, const ConfigFile &config_file) : _file_path(file_path), _config_file(config_file) {}
		~CgiHandler() {};
		std::string execute();

	private:
		std::string _file_path;
		const ConfigFile &_config_file;

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

