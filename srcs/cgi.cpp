#include "cgi.hpp"

void CgiHandler::child_is_in_orbit()
{
	ContainerToStringArray args(_args);
	ContainerToStringArray environ(_environ);
	_child_pid = execve(_cgi_path.c_str(), args.getArray(), environ.getArray());
	throw std::runtime_error("Cannot launch" + _cgi_path);
}

void CgiHandler::launch_child()
{
	// check_args_env(); //TODO: to implement

	_child_pid = fork();
	if (_child_pid == -1)
	{
		std::runtime_error("Houston we have a problem, abord launch of child");
	}
	else if (_child_pid == 0)
	{
		dup2(_child_pipe[PIPE_WRITE], STDOUT_FILENO);
		close(_child_pipe[PIPE_READ]);
		child_is_in_orbit();
	}
	else
	{
		close(_child_pipe[PIPE_WRITE]);
	}
}

std::string CgiHandler::capture_child_return()
{
	char read_buffer[CGI_BUFFER_SIZE];
	ssize_t size_read;
	std::string return_str;
	int wstatus;

	pid_t status = waitpid(-1, &wstatus, 0);
	if (status == - 1)
		std::cerr << "No child launched or error while waiting.\n";

	if (WIFEXITED(wstatus))
		std::cout << "Child return code : " << WEXITSTATUS(wstatus) << std::endl;

	do
	{
		size_read = read(_child_pipe[PIPE_READ], read_buffer, CGI_BUFFER_SIZE);
		if (size_read == -1)
			throw std::runtime_error("Could not read from file."); // TODO: add errno output
		return_str += read_buffer;
	}
	while (size_read == CGI_BUFFER_SIZE);
	return (return_str);
}

std::string CgiHandler::execute()
{
	std::string return_str;

	if (pipe(_child_pipe))
	{
		throw std::runtime_error("Could not pipe process.");
	}
	launch_child();
	
	return (return_str);
}
