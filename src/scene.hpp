#pragma once

#include <vector>
#include <iostream>
#include <list>
#include <thread>
#include <iomanip>
#include <queue>
#include <random>
#include <functional>

#include <glm/gtx/vector_angle.hpp>

#include "basic_types.hpp"
#include "shapes.hpp"
#include "lights.hpp"

#define TWO_PI 6.28318

class Scene //Let's get static B) (This is now an implicit singleton)
{
public:
	Scene();
	Color raycastScene(Ray& initialRay);
	unsigned getNCalculations() const { return _nCalculations; }

private:
	static std::vector<TriangleObj> _sceneTris;
	static std::vector<Tetrahedron> _tetrahedrons;
	static std::vector<Sphere> _spheres;
	static std::vector<PointLight> _pointLights;
	static std::vector<CeilingLight> _ceilingLights;
	static long long unsigned _nCalculations;

	static constexpr float _ambientContribution = 0.2f;

	// Checks for intersections and if so attaches intersection data to arg
	static bool rayIntersection(Ray& arg);
	//Helper method for rayIntersection
	template<typename T>
	static T* calcIntersections(std::vector<T>& container, Ray& ray, float& minT,
		std::optional<IntersectionData>& closestIntersectData, SceneObject* closestIntersectObject);

	static double shadowRayContribution(const Vertex& point, const Direction& normal);
	static bool objectIsVisible(const Ray& ray, const SceneObject& obj, const std::optional<IntersectionData>& input, const Direction& normal);
	
	//TODO these things maybe fits better in RayTree
	static constexpr float _airIndex = 1.f;
	static constexpr float _glassIndex = 1.5f;
	static Ray computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint);
	static Ray computeRefractedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject);
	static Direction computeShadowRayDirection(const Vertex& point);

	class RayTree
	{
	public:
		RayTree(Ray& initialRay);
		void raytracePixel(bool isMonteCarloTree);
		Color getPixelColor() const { return _finalColor; }

	private:
		std::unique_ptr<Ray> _head;
		Color _finalColor;
		size_t _treeSize;
		constexpr static size_t _maxTreeSize = 512;		

		//Random generator stuff for monte carlo
		constexpr static float _terminationProbability = 0.2f;
		std::mt19937 _gen;
		std::uniform_real_distribution<float> _rng;
		void monteCarloDiffuseContribution(Ray* initialRay, const IntersectionData& initialIntersection, const SceneObject* intersectObj);
		Ray generateRandomReflectedRay(const Direction& initialDirection, const Direction& normal, const Vertex& intersectPoint);

		void constructRayTree(const bool& isMonteCarloTree);
		Color traverseRayTree(Ray* input, bool isMonteCarloTree) const;

		void attachReflected(const IntersectionData& intData, Ray* currentRay) const;
		void attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay);
		void attachRefracted(const IntersectionData& intData, Ray* currentRay) const;
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