
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
    // std::cout << "host: " << config.getHost() << std::endl;
    // std::cout << "port: " << config.getPort() << std::endl;

    Server serv;
    serv.testServer(config.getHost(),  config.getPort());



    return 0;
}
