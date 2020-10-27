#pragma once

struct Config
{
	int resolution;
	int samplesPerPixel;
	bool eyeToggle;

	float monteCarloTerminationProbability;
	int numShadowRaysPerIntersection;
};
