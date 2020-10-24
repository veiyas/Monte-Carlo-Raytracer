#include <iostream>
#include <chrono>
#include <ctime>   

#include "basic_types.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "ray.hpp"

int main()
{
	//std::cout << glm::angle(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0)) << '\n';
	//std::cout << glm::degrees(glm::angle(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0))) << '\n';
	// CONCLUSION: glm::angle returns radians

	Scene testScene{};

	//Camera testCamera{ false };
	Camera testCamera{ false, 200 };
	testCamera.render(testScene);

	testCamera.sqrtAllPixels();

	testCamera.createPNG("output.png");

	// Save under unique name as well, to document progress
	auto unixTime = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	std::string filename = "Renders/MC_" + std::to_string(unixTime) + ".png";
	testCamera.createPNG(filename);

	return 0;
}