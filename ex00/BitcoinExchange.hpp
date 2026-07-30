#ifndef BITCOIN_EXCHANGE_HPP
#define BITCOIN_EXCHANGE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <map>
#include <cstdlib>

class BitcoinExchange {
    private:
    std::map<std::string, double> database;
    static bool splitInputLine(const std::string &line, std::string &date, std::string &value);
    static bool isValidDate(const std::string &date);
    static bool isValidValue(const std::string &value, double &outValue, std::string &errorMsg);
    public:
    // OCF
    BitcoinExchange();
    BitcoinExchange(const BitcoinExchange &other);
    BitcoinExchange &operator=(const BitcoinExchange &other);
    ~BitcoinExchange();
    
    // methods
    void loadDatabase(const std::string &filename);
    void processInputFile(const std::string &filename);
    // getters
    const std::map<std::string, double> &getDatabase() const;
    double getExchangeRate(const std::string &date) const;

    // exceptions
    class FailedToOpenFile : public std::exception {
        const char *what() const throw();
    };
};

#endif // BITCOIN_EXCHANGE_HPP