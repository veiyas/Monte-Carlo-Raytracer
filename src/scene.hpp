#pragma once

#include <vector>
#include <iostream>
#include <list>
#include <thread>
#include <optional>

#include "basic_types.hpp"
#include "shapes.hpp"
#include "lights.hpp"

class Scene
{
public:
	Scene();

	Color intersection(Ray& arg) const;
private:
	std::vector<Triangle> _sceneTris;
	std::vector<Tetrahedron> _tetrahedrons;
	std::vector<Sphere> _spheres;
	std::vector<PointLight> _pointLights;

	static constexpr float ambientContribution = 0.2f;

	float shadowRayContribution(const Vertex& point, const Direction& normal) const;

	bool objectIsVisible(const std::pair<float, Triangle>& input, const Direction& normal) const;
};

const std::vector<Vertex> floorVertices{
	Vertex{-3, 0, -5, 1}, Vertex{0, 0, -5, 1}, Vertex{0, 6, -5, 1},
	Vertex{-3, 0, -5, 1}, Vertex{0, -6, -5, 1}, Vertex{0, 0, -5, 1},
	Vertex{0, -6, -5, 1}, Vertex{10, 6, -5, 1}, Vertex{0, 6, -5, 1},
	Vertex{0, -6, -5, 1}, Vertex{10, -6, -5, 1}, Vertex{10, 6, -5, 1},
	Vertex{10, 0, -5, 1}, Vertex{13, 0, -5, 1}, Vertex{10, 6, -5, 1},
	Vertex{10, 0, -5, 1}, Vertex{10, -6, -5, 1}, Vertex{13, 0, -5, 1}
};

const std::vector<Vertex> ceilingVertices{
	Vertex{-3, 0, 5, 1}, Vertex{0, 0, 5, 1}, Vertex{0, 6, 5, 1},
	Vertex{-3, 0, 5, 1}, Vertex{0, -6, 5, 1}, Vertex{0, 0, 5, 1},
	Vertex{0, -6, 5, 1}, Vertex{10, 6, 5, 1}, Vertex{0, 6, 5, 1},
	Vertex{0, -6, 5, 1}, Vertex{10, -6, 5, 1}, Vertex{10, 6, 5, 1},
	Vertex{10, 0, 5, 1}, Vertex{13, 0, 5, 1}, Vertex{10, 6, 5, 1},
	Vertex{10, 0, 5, 1}, Vertex{10, -6, 5, 1}, Vertex{13, 0, 5, 1}
};

const std::vector<Vertex> wallVertices{
	Vertex{-3, 0, -5, 1}, Vertex{0, 6, -5, 1}, Vertex{0, 6, 5, 1},
	Vertex{-3, 0, -5, 1}, Vertex{0, 6, 5, 1}, Vertex{-3, 0, 5, 1}, //n = 0.894427 0.447214 0

	Vertex{0, 6, -5, 1}, Vertex{10, 6, -5, 1}, Vertex{10, 6, 5, 1},
	Vertex{0, 6, -5, 1}, Vertex{10, 6, 5, 1}, Vertex{0, 6, 5, 1}, //n = 0 -1 0

	Vertex{10, 6, -5, 1}, Vertex{13, 0, -5, 1}, Vertex{13, 0, 5, 1},
	Vertex{10, 6, -5, 1}, Vertex{13, 0, 5, 1}, Vertex{10, 6, 5, 1}, //n = -0.894427 -0.447214 0

	Vertex{13, 0, -5, 1}, Vertex{10, -6, -5, 1}, Vertex{10, -6, 5, 1},
	Vertex{13, 0, -5, 1}, Vertex{10, -6, 5, 1}, Vertex{13, 0, 5, 1}, //n = -0.894427 0.447214 0

	Vertex{0, -6, -5, 1}, Vertex{10, -6, -5, 1}, Vertex{10, -6, 5, 1},
	Vertex{0, -6, -5, 1}, Vertex{10, -6, 5, 1}, Vertex{0, -6, 5, 1}, //n = 0 1 0

	Vertex{0, -6, -5, 1}, Vertex{-3, 0, -5, 1}, Vertex{-3, 0, 5, 1},
	Vertex{0, -6, -5, 1}, Vertex{-3, 0, 5, 1}, Vertex{0, -6, 5, 1}, //n = 0.894427 0.447214 0
};

const std::vector<Direction> wallNormals{
	Direction{0.894427, -0.448214, 0},
	Direction{0, -1, 0},
	Direction{-0.894427, -0.448214, 0},
	Direction{-0.894427, 0.448214, 0},
	Direction{0, 1, 0},
	Direction{0.894427, 0.448214, 0},
};