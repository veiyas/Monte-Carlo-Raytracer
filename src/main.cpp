#include <iostream>
#include "basic_types.hpp"
#include "camera.hpp"

int main()
{
	//Ray::initVertexList();

	Camera test{ false };
	test.render();
	test.createImage();
}