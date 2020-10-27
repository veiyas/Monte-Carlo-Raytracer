#include <iostream>

#include "scene.hpp"
#include "camera.hpp"
#include <point_multiset.hpp>

struct Vertex_accessor
{
	int operator() (spatial::dimension_type dim, const Vertex& p) const
	{
		switch (dim)
		{
		case 0: return p.x;
		case 1: return p.y;
		case 2: return p.z;
		default: throw std::out_of_range("dim");
		}
	}
};

int main()
{
	//spatial::point_multiset <3, spatial::accessor_less<Vertex_accessor, Vertex>> test;
	//std::cout << glm::angle(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0)) << '\n';
	//std::cout << glm::degrees(glm::angle(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0))) << '\n';
	// CONCLUSION: glm::angle returns radians

	Scene testScene{};

	Camera testCamera{ false, 200 };
	testCamera.render(testScene);
	testCamera.createPNG();

	return 0;
}