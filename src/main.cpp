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

	// THIS IS NOT COMPLETELY WORKING RIGHT NOW
	const Config _config = {
		200,      // Resolution
		100,       // Spp
		false,    // Eye Toggle

		0.2f,     // Termination probability
		1,        // Num shadowRays per intersection
	};

	Scene scene{ _config };

	Camera testCamera{ _config };
	testCamera.render(scene);

	testCamera.sqrtAllPixels();

	// Save over old file to ease debugging
	testCamera.createPNG("output.png");

	// Save under unique name as well, to document progress
	std::string filename = "Renders/MC_" + friendlyTimeStamp() + "_" +
		std::to_string(_config.samplesPerPixel) + "spp" + ".png";
	testCamera.createPNG(filename);

	return 0;
}