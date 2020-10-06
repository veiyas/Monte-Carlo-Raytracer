#include <iostream>

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
	Camera testCamera{ false, 400 };
	testCamera.render(testScene);
	testCamera.createPNG();

	return 0;
}