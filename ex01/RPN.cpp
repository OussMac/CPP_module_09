#include "RPN.hpp"

// OCF
RPN::RPN() {}

RPN::RPN(const RPN &other) {
    (void)other;
}

RPN &RPN::operator=(const RPN &other) {
    (void)other;
    return *this;
}

RPN::~RPN() {}

const char *RPN::RPNError::what() const throw() {
    return ("Error");
}

bool RPN::isValidOperator(const std::string &str) {
    if (str == "+" || str == "-" || str == "*" || str == "/")
        return (true);
    return (false);
}

bool RPN::isValidOperand(const std::string &str) {
    char *end;
    std::strtod(str.c_str(), &end);
    if (*end == '\0')
        return (true);
    return (false);
}

int RPN::evaluate(const std::string &expression) {
    std::stack<double> stack;
    std::istringstream iss(expression);
    std::string token;

    while (iss >> token) {
        if (isValidOperator(token)) {
            if (stack.size() < 2)
                throw RPNError();

            double b = stack.top();
            stack.pop();
            double a = stack.top();
            stack.pop();

            if (token == "/" && b == 0)
                throw RPNError();

            if (token == "+") 
                stack.push(a + b);
            else if (token == "-")
                stack.push(a - b);
            else if (token == "*")
                stack.push(a * b);
            else if (token == "/")
                stack.push(a / b);

        } else if (isValidOperand(token)) {
            stack.push(std::strtod(token.c_str(), NULL));
        } else
            throw RPNError();
        // // PRINT STACK DEBUG
        // std::stack<double> tempStack = stack;
        // std::cout << "Stack: ";
        // while (!tempStack.empty()) {
        //     std::cout << tempStack.top() << " ";
        //     tempStack.pop();
        // }
        // std::cout << std::endl;
    }
    if (stack.size() != 1)
        throw RPNError();
    return stack.top();
}