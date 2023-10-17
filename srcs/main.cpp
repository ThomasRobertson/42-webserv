
#include "ConfigFile.hpp"
#include "Server.hpp"

// int	main(int argc, char **argv, char **env)
int	main(int argc, char **argv)
{

    if (argc != 2)
    {
        std::cout << "Error: could not open config file." << std::endl;
        return 0;
    }

    ConfigFile config;
    if (!config.loadDataConfigFile(argv[1]))
    {
        std::cout << "Error: could not open " << argv[1] << std::endl;
        return 0;
    }
    // UserRequest userRequest = getUserRequest("GET /scripts/script.js HTTP/1.1\n");
    // std::cout << "method: " << userRequest.method << std::endl; 
    // std::cout << "root: " << userRequest.root << std::endl; 
    std::cout << "host: " << config.getHost() << std::endl;
    std::cout << "port: " << config.getPort() << std::endl;

    Server serv;
    serv.startServer(config.getHost(),  config.getPort());

    return 0;
}
