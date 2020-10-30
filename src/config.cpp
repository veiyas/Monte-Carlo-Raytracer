#include "config.hpp"

Config& Config::instance()
{
	static Config instance;
	return instance;
}

int Config::resolution()
{
	return instance()._resolution;
}

int Config::samplesPerPixel()
{
	return instance()._samplesPerPixel;
}

bool Config::eyeToggle()
{
	return instance()._eyeToggle;
}

float Config::monteCarloTerminationProbability()
{
	return instance()._monteCarloTerminationProbability;
}

int Config::numShadowRaysPerIntersection()
{
	return instance()._numShadowRaysPerIntersection;
}

bool Config::usePhotonMapping()
{
	return instance()._usePhotonMapping;
}

void Config::setResolution(int res)
{
	_resolution = res;
}

void Config::setSamplesPerPixel(int spp)
{
	_samplesPerPixel = spp;
}

void Config::setEyeToggle(bool eye)
{
	_eyeToggle = eye;
}

void Config::setMonteCarloTerminationProbability(float prob)
{
	_monteCarloTerminationProbability = prob;
}

void Config::setNumShadowRaysPerIntersection(int num)
{
	_numShadowRaysPerIntersection = num;
}

void Config::setUsePhotonMapping(bool use)
{
	_usePhotonMapping = use;
}
