#include <sstream>
#include <string>
#include <vector>

#include "string_ex.h"

StringEx::StringEx() : std::string() {}
StringEx::StringEx(const char *str) : std::string(str) {}
StringEx::StringEx(const std::string str) : std::string(str) {}

StringEx &StringEx::operator=(const char *str) {
    std::string::operator=(str);
    return *this;
}
StringEx &StringEx::operator=(const std::string &str) {
    std::string::operator=(str);
    return *this;
}

std::vector<std::string> StringEx::Split(char sep) {
    std::vector<std::string> strs;
    std::stringstream ss(*this);
    std::string buf;

    while (std::getline(ss, buf, sep)) {
        strs.push_back(buf);
    }

    return strs;
}