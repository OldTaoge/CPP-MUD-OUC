//
// Created by Wentao on 2025/9/1.
//
#include <sstream>
#include <iostream>
#include <vector>
#include "utils.hpp"
// 辅助函数，用于按行分割字符串
std::vector<std::string> Utils::split_string(const std::string& str) {
    std::vector<std::string> lines;
    std::stringstream ss(str);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    return lines;
}