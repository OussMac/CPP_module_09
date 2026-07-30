#ifndef RPN_HPP
#define RPN_HPP

#include <iostream>
#include <sstream>
#include <exception>
#include <stack>
#include <cstdlib>

class RPN
{
    private:
    static bool isValidOperator(const std::string &str);
    static bool isValidOperand(const std::string &str);
    public:
    // OCF
    RPN();
    RPN(const RPN &other);
    RPN &operator=(const RPN &other);
    ~RPN();
    // Methods
    int evaluate(const std::string &expression);
    // exceptions
    class RPNError : public std::exception {
        const char *what() const throw();
    };
};

#endif
