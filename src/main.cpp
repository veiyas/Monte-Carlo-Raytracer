#include <iostream>
#include <chrono>
#include <ctime>   
#include <string>

#include "basic_types.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "config.hpp"
#include "util.hpp"

int main()
{
	const Config config = {
		200,      // Resolution
		10,       // Spp
		false,    // Eye Toggle

		0.2f,     // Termination probability
		1,        // Num shadowRays per intersection
	};

	Scene scene{ config };

	scene.addTetra(BRDF{ BRDF::DIFFUSE }, 1.0f, Color{ 0.0, 0.0, 1.0 }, Vertex{ 6.0f, -1.5f, -2.0f, 1.0f });
	scene.addSphere(BRDF{ BRDF::DIFFUSE }, 1.0f, Color{ 0.7, 0.2, 1.0 }, Vertex{ 5.f, 4.f, -3.5f, 1.f });
	scene.addSphere(BRDF{ BRDF::REFLECTOR }, 1.5f, Color{ 0.0, 0.0, 0.0 }, Vertex{ 7.f, -3.5f, 3.5f, 1.f });
	scene.addSphere(BRDF{ BRDF::TRANSPARENT }, 1.5f, Color{ 0.1, 0.1, 0.1 }, Vertex{ 8.f, 1.5f, -3.5f, 1.f });

	Camera testCamera{ config };
	testCamera.render(scene);

	testCamera.sqrtAllPixels();

	// Save over old file to ease debugging
	testCamera.createPNG("output.png");

	// Save under unique name as well, to document progress
	std::string filename = "Renders/MC_" + friendlyTimeStamp() + "_" +
		std::to_string(config.samplesPerPixel) + "spp" + ".png";
	testCamera.createPNG(filename);

	return 0;
}