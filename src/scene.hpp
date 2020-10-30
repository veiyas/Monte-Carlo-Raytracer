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
	Scene(const Config& conf);
	Color raycastScene(Ray& initialRay);
	unsigned getNCalculations() const { return _nCalculations; }

//<<<<<<< HEAD


	//void addSphere(BRDF brdf, float radius, Color color, Vertex position);
	//void addTetra(BRDF brdf, float radius, Color color, Vertex position);

	SceneGeometry _sceneGeometry;
	PhotonMap _photonMap;
	//Vertex _pointLight;

private:
	//std::vector<TriangleObj> _sceneTris;
	//std::vector<Tetrahedron> _tetrahedrons;
	//std::vector<Sphere> _spheres;
	//std::vector<PointLight> _pointLights;
	//std::vector<CeilingLight> _ceilingLights;
	mutable long long unsigned _nCalculations;

	const Config& _config;

	// How much reflected (and also refracted) rays starting point is offset from
	// intersection
	static constexpr float _reflectionOffset = 0.0001;

	mutable std::mt19937 _gen;
	mutable std::uniform_real_distribution<float> _rng;

	// Checks for intersections and if so attaches intersection data to arg
	//bool rayIntersection(Ray& arg) const;
	//Helper method for rayIntersection
	//template<typename T>
	//const T* calcIntersections(const std::vector<T>& container, Ray& ray, float& minT,
	//	std::optional<IntersectionData>& closestIntersectData, const SceneObject* closestIntersectObject) const;

	//Color localAreaLightContribution(const Ray& inc, const Vertex& point,
	//	const Direction& normal, const SceneObject* obj) const;
	//bool pathIsVisible(Ray& ray, const Direction& normal) const;
	//double shadowRayContribution(const Vertex& point, const Direction& normal);
	//static bool objectIsVisible(const Ray& ray, const SceneObject& obj, const std::optional<IntersectionData>& input, const Direction& normal);
	
	//TODO these things maybe fits better in RayTree
	//They def fits better as object properties, hehe
	//At leas the glass one
	static constexpr float _airIndex = 1.f;
	static constexpr float _glassIndex = 1.5f;
	//static Ray computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject);
	//static Ray computeRefractedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject);
	//Direction computeShadowRayDirection(const Vertex& point);
};

class RayTree
{
public:
	RayTree() = default;
	RayTree(Ray& initialRay, Scene* scene, const Config& conf);
	void raytracePixel();
	Color getPixelColor() const { return _finalColor; }


	//// TODO MOVE
	//Ray generateRandomReflectedRay(const Direction& initialDirection, const Direction& normal,
	//	const Vertex& intersectPoint, float rand1, float rand2);

private:
	std::unique_ptr<Ray> _head;
	Color _finalColor;
	size_t _treeSize;
	Scene* _scene;
	const Config& _config;

	constexpr static size_t _maxTreeSize = 512;

	//Random generator stuff for monte carlo
	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;
	//void monteCarloDiffuseContribution(
	//	Ray* initialRay,
	//	const IntersectionData& initialIntersection,
	//	const SceneObject* intersectObj);

	void constructRayTree();
	Color traverseRayTree(Ray* input) const;

	void attachReflected(const IntersectionData& intData, Ray* currentRay) const;
	void attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay, float rand1, float rand2);
	void attachRefracted(const IntersectionData& intData, Ray* currentRay) const;
};
