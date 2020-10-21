#pragma once

#include "ray.hpp"

using IntersectionSurface = std::pair<IntersectionData, unsigned>;
/************************
	  Declarations
************************/
static constexpr float _airIndex = 1.f;
static constexpr float _glassIndex = 1.5f;

/************************
	Implementations
************************/
template<typename T>
T* calcIntersection(std::vector<T>& objects, Ray& ray, float& minT,
	std::optional<IntersectionData>& closestIntersectData, SceneObject* closestIntersectObject)
{
	T* intersectObject = static_cast<T*>(closestIntersectObject);

	for (size_t i{ 0 }; i < objects.size(); ++i)
	{
		auto tempIntersection = objects[i].rayIntersection(ray);
		//Did intersection occur, and is it closer than minT?
		if (tempIntersection.has_value() && tempIntersection.value()._t < minT)
		{
			closestIntersectData = tempIntersection;
			intersectObject = &objects[i];
			minT = tempIntersection.value()._t;
		}
	}
	return intersectObject;
}

inline static bool rayIntersection(Ray& ray, SceneGeometry& geometry)
{
	std::optional<IntersectionData> closestIntersectData{};
	SceneObject* closestIntersectObject = nullptr;
	float minT = 1e+10;

	closestIntersectObject = calcIntersection(geometry._sceneTris, ray, minT, closestIntersectData, closestIntersectObject);
	closestIntersectObject = calcIntersection(geometry._tetrahedons, ray, minT, closestIntersectData, closestIntersectObject);
	closestIntersectObject = calcIntersection(geometry._spheres, ray, minT, closestIntersectData, closestIntersectObject);
	closestIntersectObject = calcIntersection(geometry._ceilingLights, ray, minT, closestIntersectData, closestIntersectObject);

	if (closestIntersectData.has_value())
	{
		ray.setIntersectedObject(closestIntersectObject);
		ray.setIntersectionData(closestIntersectData.value());
		return true;
	}
	return false;
}


template<typename T>
inline void calcIntersection(const std::vector<T>& objects, Ray& ray, std::vector<IntersectionSurface>& intersections)
{
	for (size_t i{ 0 }; i < objects.size(); ++i)
	{
		auto tempIntersection = objects[i].rayIntersection(ray);
		//Did intersection occur?
		if (tempIntersection.has_value())
		{
			intersections.push_back(std::make_pair(
				tempIntersection.value(),
				objects[i].accessBRDF().getSurfaceType()));
		}
	}
}
inline void photonIntersection(Ray& ray, const SceneGeometry& geometry, std::vector<IntersectionSurface>& intersections)
{
	calcIntersection(geometry._sceneTris, ray, intersections);
	calcIntersection(geometry._tetrahedons, ray, intersections);
	calcIntersection(geometry._spheres, ray, intersections);

	std::sort(intersections.begin(), intersections.end(), [](IntersectionSurface& a, IntersectionSurface& b)
		{
			return a.first._t < b.first._t;
		});
}


inline Ray computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint)
{
	Direction incomingRayDirection = incomingRay.getNormalizedDirection();
	//Angle between normal and incoming ray
	float angle = glm::angle(normal, incomingRayDirection);

	Direction reflectedDirection =
		incomingRayDirection - 2.f * (glm::dot(incomingRayDirection, normal)) * normal;

	return Ray{ Vertex{ intersectionPoint },
				Vertex{ Direction(intersectionPoint) + reflectedDirection, 1.f } };
}

inline Ray computeRefractedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject)
{
	Direction incomingDir = incomingRay.getNormalizedDirection();
	const float n1n2 = insideObject ? _glassIndex / _airIndex : _airIndex / _glassIndex;
	const float NI = glm::dot(normal, incomingDir);
	const float sqrtExpression = 1 - ((glm::pow(n1n2, 2)) * (1 - glm::pow(NI, 2)));

	Direction refractDir = n1n2 * incomingDir + normal * (-n1n2 * NI
		- glm::sqrt(sqrtExpression)
		);

	return Ray{ intersectionPoint, Vertex{ glm::vec3{ intersectionPoint } + refractDir, 1.0f } };
}

