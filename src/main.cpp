#include <iostream>

#include "basic_types.hpp"
#include "scene.hpp"
#include "camera.hpp"

int main()
{
	Ray::initVertexList();
	Scene test{};

	Camera testCamera{ false };
	testCamera.render();
	testCamera.createImage();

	return 0;
}