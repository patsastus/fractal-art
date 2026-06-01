#include "utils.hpp"
#include <iostream>
#include <stdexcept>

uint32_t sawtooth(uint32_t i, uint32_t max) {
    uint32_t half = max / 2;
    uint32_t quarter = max / 4;
    if (quarter == 0) return 0;
    if (i % half < quarter)
        return i % quarter;
    else
        return quarter - (i % quarter);
}

bool parse_double(const std::string& s, double& out) {
    try {
        size_t pos = 0;
        out = std::stod(s, &pos);
        return pos == s.size();  // entire string consumed
    } catch (const std::exception&) {
        return false;
    }
}

void print_usage() {
    std::cerr << "Please provide valid input:\n"
              << "./fractol [jmn] [args...] [-i <iterations>]\n"
              << "Example:\t./fractol m\n"
              << "\t\t./fractol j -0.4 0.6\n"
              << "\t\t./fractol n\n"
              << "\t\t./fractol m -i 500\n";
}
