#pragma once

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>
#include <map>

#include "utils.hpp"

#define PIPE_READ 0
#define PIPE_WRITE 1

#define CGI_BUFFER_SIZE 1024

class CgiHandler
{
	public:
		struct cgi_env
		{
			int i;
		};
		

		CgiHandler(std::string cgi_path, std::vector<std::string> args) : _cgi_path(cgi_path), _args(args) {}
		~CgiHandler() {};
		std::string execute();

	private:
		std::string _cgi_path;
		std::vector<std::string> _args;
		std::map<std::string, std::string> _environ;

		pid_t _child_pid;
		int _child_pipe[2];

		// char **build_args(std::vector<std::string> args); //? Still needed ?
		void launch_child();
		void child_is_in_orbit();
		std::string capture_child_return();
		void check_args_env() const;


};

