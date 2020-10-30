#pragma once

#include <functional>

#include "basic_types.hpp"

// Test if func evaluates to true for all components of col
bool allComponents(Color col, std::function<bool(double)> func);

// Test if func evaluates to true for at least one of col's components
bool someComponent(Color col, std::function<bool(double)> func);

std::string friendlyTimeStamp();

std::string durationFormat(std::chrono::duration<double> duration);
