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

#include "basic_types.hpp"
#include "shapes.hpp"
#include "config.hpp"
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
	std::unique_ptr<PhotonMap> _photonMap;

private:
	mutable long long unsigned _nCalculations;

	// How much reflected (and also refracted) rays starting point is offset from
	// intersection
	static constexpr float _reflectionOffset = 0.0001;

	mutable std::mt19937 _gen;
	mutable std::uniform_real_distribution<float> _rng;
	
	//TODO these things maybe fits better in RayTree
	//They def fits better as object properties, hehe
	//At leas the glass one
	static constexpr float _airIndex = 1.f;
	static constexpr float _glassIndex = 1.5f;
};

class RayTree
{
public:
	RayTree() = default;
	RayTree(Ray& initialRay, Scene* scene);
	void raytracePixel();
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

	void constructRayTree();
	Color traverseRayTree(Ray* input, bool hasBeenDiffuselyReflected) const;

	void attachReflected(const IntersectionData& intData, Ray* currentRay) const;
	void attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay, float rand1, float rand2);
	void attachRefracted(const IntersectionData& intData, Ray* currentRay) const;
};
