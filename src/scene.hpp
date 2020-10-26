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
#include <glm/gtx/string_cast.hpp>

#include "photonmap.hpp"
#include "raycastingfunctions.hpp"
#include "scenegeometry.hpp"

class Scene
{
public:
	Scene();
	Color raycastScene(Ray& initialRay);
	unsigned getNCalculations() const { return _nCalculations; }

	SceneGeometry _sceneGeometry;
	PhotonMap _photonMap;
	Vertex _pointLight;
	static long long unsigned _nCalculations;
private:

	static constexpr float _ambientContribution = 0.2f;
	//static double shadowRayContribution(const Vertex& point, const Direction& normal);
	//static bool objectIsVisible(const Ray& ray, const SceneObject& obj, const std::optional<IntersectionData>& input, const Direction& normal);
	//static Direction computeShadowRayDirection(const Vertex& point);	
};

class RayTree
{
public:
	RayTree() = default;
	RayTree(Ray& initialRay, Scene* scene);
	void raytracePixel(bool isMonteCarloTree);
	Color getPixelColor() const { return _finalColor; }

private:
	std::unique_ptr<Ray> _head;
	Color _finalColor;
	size_t _treeSize;
	Scene* _scene;

	constexpr static size_t _maxTreeSize = 512;

	//Random generator stuff for monte carlo
	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;
	void monteCarloDiffuseContribution(
		Ray* initialRay,
		const IntersectionData& initialIntersection,
		const SceneObject* intersectObj);

	void constructRayTree(const bool& isMonteCarloTree);
	Color traverseRayTree(Ray* input, bool isMonteCarloTree) const;

	void attachReflected(const IntersectionData& intData, Ray* currentRay) const;
	void attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay);
	void attachRefracted(const IntersectionData& intData, Ray* currentRay) const;
};

