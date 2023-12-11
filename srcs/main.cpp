
#include "ConfigFile.hpp"
#include "Server.hpp"
#include "StartServers.hpp"

bool EXIT_G = false;

void signalHandler(int signum)
{
    (void)signum;
    std::cout << " [i] Server shutted down." << std::endl;
    EXIT_G = true;
}

int	main(int argc, char **argv)
{
    if (argc == 1)
    {
        std::cout << "Default Config File" << std::endl;
        argv[1] = const_cast<char*>("config/config.conf");
    }
    else if (argc != 2)
    {
        std::cout << "Error: could not open config file." << std::endl;
        return 1;
    }

    ConfigFile configFile;

    if (!configFile.loadDataConfigFile(argv[1]))
    {
        std::cout << "Error: could not open " << argv[1] << std::endl;
        return 1;
    }

    signal(SIGINT, signalHandler);

    StartServers servers(configFile);
    try
    {
        servers.createServers();
        servers.initServers();
    }
	catch(const std::exception& e) {std::cerr << e.what() << std::endl;}

    servers.closeServers();

    return 0;
}
