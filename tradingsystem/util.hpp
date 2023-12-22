#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <cmath> // For round function
#include <iomanip>


//function to convert decimal to fractional representation - by chatgpt
std::string decimal_to_fractional(double decimal) {
    int whole_part = static_cast<int>(decimal);
    double fractional_part = decimal - whole_part;

    // Convert the fractional part to 32nds
    double fraction_32 = fractional_part * 32;
    int fraction_32_whole = static_cast<int>(fraction_32);
    double fraction_32_remainder = fraction_32 - fraction_32_whole;

    // Convert the remainder to 256ths
    int fraction_256 = static_cast<int>(round(fraction_32_remainder * 8)); // 8 is 256/32

    // Handle special case for 4 (represented as '+')
    std::string fraction_256_str = (fraction_256 == 4) ? "+" : (fraction_256 != 0 ? std::to_string(fraction_256) : "");

    return std::to_string(whole_part) + "-" + (fraction_32_whole < 10 ? "0" : "") + std::to_string(fraction_32_whole) + fraction_256_str;
}

//function to convert fractional to decimal representation - by chatgpt
double fractional_to_decimal(const std::string& fractional)
{
    size_t dashPos = fractional.find('-');
    if (dashPos == std::string::npos) {
        std::cerr << "Invalid format" << std::endl;
        return -1.0; // or handle error appropriately
    }

    // Extract whole part and fractional part
    std::string wholePart = fractional.substr(0, dashPos);
    std::string fractionPart = fractional.substr(dashPos + 1);

    // Extract parts for 32nds and 256ths
    int fraction32 = std::stoi(fractionPart.substr(0, 2));
    int fraction256 = 0;
    if (fractionPart.length() > 2) {
        fraction256 = (fractionPart[2] == '+') ? 4 : std::stoi(fractionPart.substr(2));
    }

    // Convert to decimal
    float decimalFraction = fraction32 / 32.0f + fraction256 / 256.0f;

    return std::stof(wholePart) + decimalFraction;
}

#endif // !UTIL_HPP
