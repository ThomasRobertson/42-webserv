
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

    if (argc != 2)
    {
        std::cout << "Error: could not open config file." << std::endl;
        return 0;
    }

    ConfigFile configFile;

    if (!configFile.loadDataConfigFile(argv[1]))
    {
        std::cout << "Error: could not open " << argv[1] << std::endl;
        return 0;
    }

    signal(SIGINT, signalHandler);

    StartServers servers(configFile);
    servers.createServers();
    servers.initServers();

    return 0;
}
