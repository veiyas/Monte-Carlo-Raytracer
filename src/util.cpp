#include "util.hpp"

// Test if func evaluates to true for all components of col
bool allComponents(Color col, std::function<bool(double)> func)
{
	return func(col.r) && func(col.g) && func(col.b);
}

// Test if func evaluates to true for at least one of col's components
bool someComponent(Color col, std::function<bool(double)> func)
{
	return func(col.r) || func(col.g) || func(col.b);
}