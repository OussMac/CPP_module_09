#include "BitcoinExchange.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Error: could not open file." << std::endl;
        return (1);
    }
    try {
        BitcoinExchange be;
        be.loadDatabase("test.csv");
        be.processInputFile(argv[1]);
        if (!be.getDatabase().empty() && true)
        {
            std::cout << "Database loaded successfully." << std::endl;
            // print database contents for debugging
            for (std::map<std::string, double>::const_iterator it = be.getDatabase().begin(); 
                it != be.getDatabase().end(); ++it)
            {
                std::cout << it->first << " => " << it->second << std::endl;
            }
            /* debugging */
                
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return (0);
}