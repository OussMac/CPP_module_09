#include "BitcoinExchange.hpp"
//OCF
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
// exception
const char *BitcoinExchange::FailedToOpenFile::what() const throw() {
    return "Error: could not open file.";
}
// getters
const std::map<std::string, double> &BitcoinExchange::getDatabase() const {
    return this->database;
}

void BitcoinExchange::loadDatabase(const std::string &filename) {
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

static std::string trim(const std::string &str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }
    return str.substr(start, end - start);
}

bool BitcoinExchange::splitInputLine(const std::string &line, std::string &date, std::string &value) {
    size_t npos = line.find('|');
    if (npos == std::string::npos)
        return false;
    date = trim(line.substr(0, npos));
    value = trim(line.substr(npos + 1));
    return true;
}

bool BitcoinExchange::isValidDate(const std::string &date) {
    if (date.length() != 10)
        return false;
    if (date[4] != '-' || date[7] != '-')
        return false;
    for (size_t i = 0; i < date.length(); ++i) {
        if (i == 4 || i == 7)
            continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i])))
            return false;
    }
    int month = std::atoi(date.substr(5, 2).c_str());
    int day = std::atoi(date.substr(8, 2).c_str());
    if (month < 1 || month > 12)
        return false;
    if (day < 1 || day > 31)
        return false;
    return true;
}

bool BitcoinExchange::isValidValue(const std::string &value, double &outValue, std::string &errorMsg) {
    if (value.empty()) {
        errorMsg = "bad input";
        return false;
    }
    char *endPtr;
    double parsedValue = std::strtod(value.c_str(), &endPtr);
    if (*endPtr != '\0') {
        errorMsg = "bad input";
        return false;
    }
    if (parsedValue < 0) {
        errorMsg = "not a positive number.";
        return false;
    }
    if (parsedValue > 1000) {
        errorMsg = "too large a number.";
        return false;
    }
    outValue = parsedValue;
    return true;
}

double BitcoinExchange::getExchangeRate(const std::string &date) const {
    std::map<std::string, double>::const_iterator it = this->database.lower_bound(date);
    if (it != this->database.end() && it->first == date)
        return it->second;
    if (it == this->database.begin())
        return -1;
    --it;
    return it->second;
}

void BitcoinExchange::processInputFile(const std::string &filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw FailedToOpenFile();
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line))
    {
        std::string date;
        std::string value;
        if (!splitInputLine(line, date, value)) {
            std::cerr << "Error: bad input => " + line + "\n";
            continue ;
        }
        if (!isValidDate(date)) {
            std::cerr << "Error: invalid date => " + date + "\n";
            continue ;
        }
        double inputValue;
        std::string errorMsg;
        if (!isValidValue(value, inputValue, errorMsg)) {
            std::cerr << "Error: " << errorMsg << "\n";
            continue;
        }
        double exchangeRate = getExchangeRate(date);
        if (exchangeRate < 0) {
            std::cerr << "Error: no exchange rate available for date => " + date + "\n";
            continue ;
        }
        double result = inputValue * exchangeRate;
        std::cout << date << " => " << value << " = " << result << std::endl;
    }
}