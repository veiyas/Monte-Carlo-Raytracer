#include <iostream>

#include "basic_types.hpp"
#include "scene.hpp"
#include "camera.hpp"

int main()
{
	Ray::initVertexList();
	Scene testScene{};

	Camera testCamera{ false };
	testCamera.render(testScene);
	testCamera.createImage();

	return 0;
}