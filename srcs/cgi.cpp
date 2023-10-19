#include "cgi.hpp"

//why should I use pipe ?

pid_t CgiHandler::launch_child()
{
	check_args_env();
	pid_t pid = fork();
	if (pid == -1)
		perror("launch_child_process: Cannot launch child");
	else if (pid == 0)
	{
		VectorToStringArray args(_args);
		pid = execve(_cgi_path.c_str(), args.getArray(), _environ);
		throw std::runtime_error("Cannot launch" + _cgi_path);
	}
	return (pid);
}

void CgiHandler::execute()
{
	pid_t child_pid;

	try
	{
		child_pid = launch_child();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		if (child_pid != 0)
			exit(EXIT_FAILURE);
	}
	
}
