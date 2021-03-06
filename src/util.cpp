#include "util.hpp"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

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

std::string friendlyTimeStamp()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");

	return oss.str();
}

std::string durationFormat(std::chrono::duration<double> duration)
{
	double elapsedTime = duration.count();
	int hoursElapsed = elapsedTime / (60 * 60);
	int minutesElapsed = (int(elapsedTime) % (60 * 60)) / 60;
	int secondsElapsed = int(elapsedTime) % 60;
	
	return std::to_string(hoursElapsed) + "h:"
	     + std::to_string(minutesElapsed) + "m:"
	     + std::to_string(secondsElapsed) + "s";
}

std::string friendlyDurationFormat(std::chrono::duration<double> duration)
{
	double elapsedTime = duration.count();
	int hoursElapsed = elapsedTime / (60 * 60);
	int minutesElapsed = (int(elapsedTime) % (60 * 60)) / 60;
	int secondsElapsed = int(elapsedTime) % 60;

	return std::to_string(hoursElapsed) + "h-"
		+ std::to_string(minutesElapsed) + "m-"
		+ std::to_string(secondsElapsed) + "s";
}
