#pragma once

class Config
{
public:
	Config(const Config&) = delete;

	static Config& instance();

	static int resolution();
	static int samplesPerPixel();
	static bool eyeToggle();

	static float monteCarloTerminationProbability();
	static int numShadowRaysPerIntersection();
	
	static bool usePhotonMapping();

	void setResolution(int res);
	void setSamplesPerPixel(int spp);
	void setEyeToggle(bool eye);
	void setMonteCarloTerminationProbability(float prob);
	void setNumShadowRaysPerIntersection(int num);
	void setUsePhotonMapping(bool use);

private:
	Config() {}

	// Default values

	int _resolution = 800;
	int _samplesPerPixel = 100;
	bool _eyeToggle = false;

	float _monteCarloTerminationProbability = 0.2f;
	int _numShadowRaysPerIntersection = 1;

	bool _usePhotonMapping = true;
};
