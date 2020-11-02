#include <iostream>
#include <chrono>
#include <ctime>   
#include <string>

#include "scene.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "config.hpp"
#include "util.hpp"

int main()
{
	auto& config = Config::instance();
	config.setResolution(200);
	config.setSamplesPerPixel(5000);
	config.setMonteCarloTerminationProbability(0.2f);
	config.setNumShadowRaysPerIntersection(1);
	config.setUsePhotonMapping(false);

	Scene scene{};
	Camera testCamera;
	testCamera.render(scene);

	testCamera.sqrtAllPixels();

	// Save over old file to ease debugging
	testCamera.createPNG("output.png");

	// Save under unique name as well, to document progress
	std::string filename = "Renders/MC_" + friendlyTimeStamp() + "_" +
		std::to_string(Config::samplesPerPixel()) + "spp" + ".png";
	testCamera.createPNG(filename);

	return 0;
}