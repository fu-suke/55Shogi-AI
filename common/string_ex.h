#pragma once
#include <string>
#include <vector>

class StringEx : public std::string {
  public:
    StringEx();
    StringEx(const char *str);
    StringEx(const std::string str);
    std::vector<std::string> Split(char sep = ' ');
    StringEx &operator=(const char *str);
    StringEx &operator=(const std::string &str);
};
