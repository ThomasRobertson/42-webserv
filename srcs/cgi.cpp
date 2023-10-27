#include "cgi.hpp"

void CgiHandler::build_args_env()
{
	// --- SERVER ENVIRON ---

	_environ["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
	_environ["SERVER_NAME"] = _config_file.getServerName(); //TODO : not in ConfigFile
	_environ["GATEWAY_INTERFACE"] = GATEWAY_INTERFACE;
	_environ["DOCUMENT_ROOT"] = _config_file.getRootDir(); //TODO : not in ConfigFile

	// --- REQUEST ENVIRON ---

	_environ["SERVER_PROTOCOL"] = SERVER_PROTOCOL; //? Is it HTTP or does it change ?
	_environ["SERVER_PORT"] = _config_file.getPort(); //?From request or server ?
	// _environ["REQUEST_METHOD"] = "" //TODO: Get request method
	_environ["PATH_INFO"] = _file_path;
	_environ["PATH_TRANSLATED"] = _file_path;
	_environ["SCRIPT_NAME"] = _cgi_path;
	// _environ["QUERY_STRING"] =""; //Todo
	_environ["REMOTE_HOST"] = ""; //? Can we leave it empty ?
	// _environ["REMOTE_ADDR"] = ""; //TODO : IP adress of client
	// _environ["AUTH_TYPE"] = AUTH_TYPE;
	// _environ["REMOTE_USER"] = "";
	// _environ["REMOTE_IDENT"] = ""; //? AUTH TYPE, REMOTE USER AND IDENT just if renable by server, implementation needed ?
	if (_environ["REQUEST_METHOD"] == "POST")
	{
		// _environ["CONTENT_TYPE"] = ""; //TODO: From request
		// _environ["CONTENT_LENGTH"] = ""; //TODO: From request
	}
	_environ["REQUEST_URI"] = _file_path;
	
	// --- CLIENT ENVIRON ---

	// _environ["HTTP_ACCEPT"] = ""; //Todo: what MIME can the client accept, from client. Needed ?
	// _environ["HTTP_ACCEPT_LANGUAGE"] = ""; //Todo: Language accepted, from client. Needed ?
	// _environ["HTTP_USER_AGENT"] = "" //Todo: User agent, from client. Needed ?
	_environ["HTTP_COOKIE"] = ""; //?Do I have to define it?
}

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
		std::cout << "Child return code : " << WEXITSTATUS(wstatus) << std::endl; //! For debug only, to be removed.

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
