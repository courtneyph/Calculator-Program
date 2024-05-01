#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <map>
#include <sstream>
#include <cassert>


using namespace std;

char const number = '8';    // a floating-point number
char const quit = 'q';      // an exit command
char const print = ';';     // a print command
char const name =  'c';     // a variable name
char const assign = '=';    // assignment command

std::map<std::string, double> predefinedVals = {
        {"pi", 3.14159},
        {"e", 2.71828}
};

class token {
    char kind_;
    double value_;
    std::string name_;

public:
    token(char ch)
      : kind_(ch), value_(0) {}
    token(double val)
      : kind_(number), value_(val) {}
    token(char ch, std::string n)
      : kind_(ch), value_(0), name_(n) {}

    char kind() const { return kind_; }
    double value() const { return value_; }
    std::string name() const { return name_; }
};

std::string const prompt = "> ";
std::string const result = "= ";

class token_stream {
    bool full;
    token buffer;
    std::istream& input;

public:
    token_stream(std::istream& is) : full(false), buffer('\0'), input(is) {}
    token get();
    void putback(token t);
    void ignore(char c);
};


void token_stream::putback(token t) {
    if (full) throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get() {
    if (full) {
        full = false;
        return buffer;
    }

    char ch;
    input >> ch;

    switch (ch) {
    case '(':
    case ')':
    case ';':
    case 'q':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
        return token(ch);
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        input.putback(ch);
        double val;
        input >> val;
        return token(val);
    default:
        if (isalpha(ch)) {
            string s;
            s += ch;
            while (input.get(ch) && (isalnum(ch) || ch == '_')) {
                s += ch;
            }
            input.putback(ch);
            return token(name, s);
        }
        if (ch == '=') {
            return token(assign);  // Ensure '=' is recognized as an assignment
        }
        throw runtime_error("Bad token: " + string(1, ch));
    }
}
void token_stream::ignore(char c) {
    if (full && c == buffer.kind()) {
        full = false;
        return;
    }
    full = false;
    char ch;
    while (input >> ch)
        if (ch == c) return;
}

double expression(token_stream& ts); // forward declaration

double primary(token_stream& ts) {
     token t = ts.get();
    switch (t.kind()) {
    case number:
        return t.value();
    case name:
        {
            token next = ts.get();
            if (next.kind() == assign) {
                double value = expression(ts); // Handle the right side of the assignment
                predefinedVals[t.name()] = value;
                return value;
            } else {
                ts.putback(next);
                if (predefinedVals.find(t.name()) != predefinedVals.end()) {
                    return predefinedVals[t.name()];
                } else {
                    throw runtime_error("Undefined variable: " + t.name());
                }
            }
        }
    case '-':
        return -primary(ts);
    case '(':
    {
        double d = expression(ts);
        t = ts.get();
        if (t.kind() != ')') throw runtime_error("')' expected");
        return d;
    }
    default:
        throw runtime_error("Primary expected");
    }
}
double term(token_stream& ts) {
    double left = primary(ts);
    while (true) {
        token t = ts.get();
        switch (t.kind()) {
        case '*':
            left *= primary(ts);
            break;
        case '/':
        {
            double d = primary(ts);
            if (d == 0) {
             std::cerr << "Error: Division by zero.\n";
                return std::numeric_limits<double>::infinity(); // Handling division by zero
            }
            left /= d;
            break;
        }
        default:
            ts.putback(t);
            return left;
        }
    }
}

double expression(token_stream& ts) {
    double left = term(ts);
    while (true) {
        token t = ts.get();
        switch (t.kind()) {
        case '+':
            left += term(ts);
            break;
        case '-':
            left -= term(ts);
            break;
        case '%':
        {
            double d = primary(ts);
            if (d == 0) throw std::runtime_error("modulo by zero");
            left = std::fmod(left, d);
            break;
        }
        default:
            ts.putback(t);
            return left;
        }
    }
}

void clean_up_mess(token_stream& ts) {
    ts.ignore(print);
}

void calculate(token_stream& ts) {
    while (true) {
        try {
            std::cout << prompt;
            token t = ts.get();

            while (t.kind() == print) t = ts.get();

            if (t.kind() == quit) return;

            ts.putback(t);
            std::cout << result << expression(ts) << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            clean_up_mess(ts);
        }
    }
}
void test_calculator(const std::string& test_label, const std::string& input_data, double expected) {
    std::stringstream input(input_data); // Input for the calculator
    std::stringstream output;            // Output from the calculator

    token_stream ts(input);
    try {
        while (true) {
            std::cout << prompt; // Normally you might print the prompt to std::cout
            token t = ts.get();
            if (t.kind() == print) continue;
            if (t.kind() == quit) break;

            ts.putback(t);
            double result = expression(ts);
            output << result; // Capture the output instead of printing it
            char ch;
            if (!(input >> ch) || ch == ';') break; // Stop at the end of each test case
        }
        double result;
        output >> result;
        assert(fabs(result - expected) < 1e-6); // Check if the result is close enough to expected
        std::cout << "Test passed: " << test_label << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << test_label << " with exception " << e.what() << "\n";
    }
}
void run_tests() {
    std::cout << "Starting tests...\n";

    test_calculator("Addition Test", "2 + 3;", 5);
    test_calculator("Subtraction Test", "5 - 3;", 2);
    test_calculator("Multiplication Test", "4 * 5;", 20);
    test_calculator("Division Test", "20 / 4;", 5);
    test_calculator("Complex Expression Test", "2 + 3 * (5 - 2);", 11);
    test_calculator("Divide by Zero Test", "10 / 0;", std::numeric_limits<double>::infinity()); // Should handle divide by zero
    test_calculator("Modulo Test", "10 % 3;", 1);
    test_calculator("Negative Numbers Test", "-5 * -2;", 10);
   // test_calculator("Variable Assignment Test", "x = 5; x + 2;", 7);

    std::cout << "Tests completed.\n";
}

int main() {
        run_tests();
        std::cout << "Begin using the calculator!" << endl;
        try {
            token_stream ts{std::cin};
            calculate(ts);
            return 0;
        } catch (std::exception const& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Exception occurred." << std::endl;
            return 2;
        }
    }
