#ifndef CALC_H
#define CALC_H

#include <string>
#include <tuple>

// Evaluate expr.
// When successful, returns the result and 0 or empty string and the error line number
std::tuple<std::string, int> EvaluateExpression(const std::string &expr);

#endif
