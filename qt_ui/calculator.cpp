#include "calculator.h"
#include "tinyexpr.h"
#include <tuple>

std::tuple<std::string, int> EvaluateExpression(const std::string &expr)
{
    int err = 0;
    double r = te_interp(expr.c_str(), &err);
    return std::make_tuple(std::to_string(r), err);
}
