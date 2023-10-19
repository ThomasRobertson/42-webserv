#pragma ounce

#include <string>
#include <unistd.h>
#include <vector>
#include <iostream>

#include "utils.hpp"


class CgiHandler
{
	public:
		struct cgi_env
		{
			int i;
		};
		

		CgiHandler(std::string cgi_path, std::vector<std::string> args, char **environ) : _cgi_path(cgi_path), _args(args), _environ(environ) {}
		~CgiHandler() {};
		void execute();

	private:
		std::string _cgi_path;
		std::vector<std::string> _args;
		char **_environ;

		char **build_args(std::vector<std::string> args);
		pid_t launch_child();
		void check_args_env() const;


};

