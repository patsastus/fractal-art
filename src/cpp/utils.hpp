#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <string>

// Makes a sawtooth wave pattern: 2 peaks, 2 valleys over [0, max)
uint32_t sawtooth(uint32_t i, uint32_t max);

// Parse a string to double with validation.
// Returns true on success, stores result in *out.
bool parse_double(const std::string& s, double& out);

// Print usage instructions to stderr
void print_usage();

#endif // UTILS_HPP
