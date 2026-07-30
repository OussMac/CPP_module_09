#include "BitcoinExchange.hpp"

BitcoinExchange::BitcoinExchange() : database() {}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &other) : database(other.database) {
    (void)other;
}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &other) {
    if (this != &other) {
        this->database = other.database;
    }
    return *this;
}

BitcoinExchange::~BitcoinExchange() {}

const char *BitcoinExchange::FailedToOpenFile::what() const throw() {
    return "Error: could not open file.";
}

const std::map<std::string, double> &BitcoinExchange::getDatabase() const {
    return this->database;
}

void BitcoinExchange::loadDatabase(const std::string &filename) {
    (void)filename;
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw FailedToOpenFile();
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        size_t npos = line.find(',');
        if (npos == std::string::npos) 
            continue;
        std::string date = line.substr(0, npos);
        double value = std::strtod(line.substr(npos + 1).c_str(), NULL);
        this->database.insert(std::make_pair(date, value));
    }
    file.close();
}

void BitcoinExchange::processInputFile(const std::string &filename) {
    (void)filename;
    // Implementation for processing the input file
    // This function should read the input file and perform necessary operations based on the loaded database
}