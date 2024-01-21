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

#ifdef UNIT_TEST_STRING_EX
#include <iostream>

int main(int argc, char **argv) {
    StringEx str1 = std::string("test0 test1 test2");
    std::cout << "str1=" + str1 << std::endl;
    std::vector<std::string> str1s = str1.Split();
    std::cout << "str1s.size()=" << str1s.size() << std::endl;
    for (auto s : str1s)
        std::cout << s << std::endl;

    StringEx str2 = std::string("test0,test1,test2,test3");
    std::cout << "str2=" + str2 << std::endl;
    std::vector<std::string> str2s = str2.Split(',');
    std::cout << "str2s.size()=" << str2s.size() << std::endl;
    for (auto s : str2s)
        std::cout << s << std::endl;

    return 0;
}
#endif // UNIT_TEST_STRING_EX
