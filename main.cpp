
#include "ConfigFile.hpp"

// int	main(int argc, char **argv, char **env)
int	main(int argc, char **argv)
{

    if (argc != 2)
    {
        std::cout << "Error: could not open config file." << std::endl;
        return 0;
    }
    
    ConfigFile btc;
    if (!btc.loadDataConfigFile(argv[1]))
    {
        std::cout << "Error: could not open " << argv[1] << std::endl;
        return 0;
    }
    


    return 0;
}
