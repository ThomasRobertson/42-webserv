
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

    ConfigFile configFile;

    if (!configFile.loadDataConfigFile(argv[1]))
    {
        std::cout << "Error: could not open " << argv[1] << std::endl;
        return 0;
    }

    Server serv(configFile);
    serv.startServer();

    return 0;
}
