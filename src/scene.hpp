#pragma once

#include <vector>
#include <iostream>
#include <list>
#include <thread>
#include <iomanip>
#include <queue>

#include <glm/gtx/vector_angle.hpp>

#include "basic_types.hpp"
#include "shapes.hpp"
#include "lights.hpp"

class Scene
{
public:
	Scene();

	Color raycastScene(Ray& initialRay);
	unsigned getNCalculations() const { return _nCalculations; }
private:
	std::vector<Triangle> _sceneTris;
	std::vector<Tetrahedron> _tetrahedrons;
	std::vector<Sphere> _spheres;
	std::vector<PointLight> _pointLights;

	static constexpr float _ambientContribution = 0.2f;
	static constexpr float _airIndex = 1.f;
	static constexpr float _glassIndex = 1.5f;
	long long unsigned _nCalculations = 0;

	std::pair<Triangle, unsigned> rayIntersection(Ray& arg);
	double shadowRayContribution(const Vertex& point, const Direction& normal) const;
	bool objectIsVisible(const std::pair<float, Triangle>& input, const Direction& normal) const;
	//TODO these can probably be static
	Ray computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint) const;
	Ray computeRefractedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject) const;

	class RayTree
	{
	public:
		RayTree(Ray& initialRay);
		void raytrace(Scene& scene);
		Color getPixelColor() const { return _finalColor; }

	private:
		std::unique_ptr<Ray> _head;
		Color _finalColor;
		size_t _treeSize;

		constexpr static size_t maxTreeSize = 512;
		constexpr static double _transmissionContrib = 0.99;
		double _reflectionContrib;
		
		void constructRayTree(Scene& scene) const;
		Color traverseRayTree(const Scene& scene, Ray* input) const;
	};
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