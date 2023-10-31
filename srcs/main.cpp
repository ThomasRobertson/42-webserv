
#include "ConfigFile.hpp"
#include "Server.hpp"
#include "StartServers.hpp"
#include <csignal>

bool EXIT_G = false;

void signalHandler(int signum)
{
    EXIT_G = true;
}

int	main(int argc, char **argv)
{

    if (argc != 2)
    {
        std::cout << "Error: could not open config file." << std::endl;
        return 0;
    }
    signal(SIGINT, signalHandler);
    ConfigFile configFile;

    if (!configFile.loadDataConfigFile(argv[1]))
    {
        std::cout << "Error: could not open " << argv[1] << std::endl;
        return 0;
    }

    StartServers servs(configFile);
    servs.createServers();
    servs.initServers();

    return 0;
}
