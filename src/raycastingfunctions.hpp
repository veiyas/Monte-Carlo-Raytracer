#pragma once

#include "ray.hpp"
#include "scenegeometry.hpp"

using IntersectionSurface = std::pair<IntersectionData, unsigned>;
/************************
	  Declarations
************************/
static constexpr float PI = 3.1415f;
static constexpr float TWO_PI = 6.28318f;
static constexpr float _airIndex = 1.f;
static constexpr float _glassIndex = 1.5f;
static constexpr float _terminationProbability = 0.2f;

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

//Fills intersections with data and sorts increasingly by distance from photon origin (AKA closest first)
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

	Ray result = Ray{ intersectionPoint, Vertex{ glm::vec3{ intersectionPoint } + refractDir, 1.0f } };
	result.setInsideObject(!incomingRay.isInsideObject());

	return result;
}

//Calculates n1, n2, reflectionCoeff and returns if ray is transmitted or not
inline bool shouldRayTransmit(double& n1, double& n2, double& reflectionCoeff, float incAngle, Ray& currentRay)
{
	if (currentRay.isInsideObject())
		n1 = _glassIndex, n2 = _airIndex;
	else
		n1 = _airIndex, n2 = _glassIndex;

	float brewsterAngle = asin(_airIndex / _glassIndex); // In radians
	if (currentRay.isInsideObject() && incAngle > brewsterAngle) // Total internal reflection
	{
		reflectionCoeff = 1.f;
		return false;
	}
	else
	{
		double R0 = pow((n1 - n2) / (n1 + n2), 2);
		reflectionCoeff = R0 + (1 - R0) * pow(1.0 - cos(incAngle), 5);
		return true;
	}
}

inline float randAzimuth(std::mt19937& _gen, std::uniform_real_distribution<float>& _rng)
{
	return TWO_PI * _rng(_gen);
}
inline float randInclination(std::mt19937& _gen, std::uniform_real_distribution<float>& _rng)
{
	return glm::asin(glm::sqrt(_rng(_gen)));
}

inline Ray generateRandomReflectedRay(
	const Direction& initialDirection,
	const Direction& normal,
	const Vertex& intersectPoint,
	std::mt19937& _gen,
	std::uniform_real_distribution<float>& _rng)
{
	//Determine local coordinate system and transformations matrices for it
	glm::vec3 Z{ normal };
	const glm::vec3 X = glm::normalize(initialDirection - glm::dot(initialDirection, Z) * Z);
	const glm::vec3 Y = glm::cross(-X, Z);
	/* GLM: Column major order
	 0  4  8  12
	 1  5  9  13
	 2  6 10  14
	 3  7 11  15
	*/
	const glm::mat4 toLocalCoord =
		glm::mat4{
		X.x, Y.x, Z.x, 0.f,
		X.y, Y.y, Z.y, 0.f,
		X.z, Y.z, Z.z, 0.f,
		0.f, 0.f, 0.f, 1.f } *
		glm::mat4{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		-intersectPoint.x, -intersectPoint.y, -intersectPoint.z, 1.f
	};
	const glm::mat4 toGlobalCoord = glm::inverse(toLocalCoord);

	//Generate random azimuth (phi) and inclination (theta)
	const float x = glm::cos(randAzimuth(_gen, _rng)) * glm::sin(randInclination(_gen, _rng));
	const float y = glm::sin(randAzimuth(_gen, _rng)) * glm::sin(randInclination(_gen, _rng));
	const float z = glm::cos(randInclination(_gen, _rng));

	//OK So this works...
	Z = glm::rotateX(Z, randInclination(_gen, _rng));
	//Z = glm::rotateY(Z, randInclination(_gen, _rng));
	Z = glm::rotateZ(Z, randAzimuth(_gen, _rng));

	const glm::vec4 globalReflected = toGlobalCoord * glm::vec4{ glm::normalize(glm::vec3(x, y, z)), 1.f };

	//Debug helper
	//const glm::vec3 globalDirection{ glm::normalize(glm::vec3{globalReflected.x, globalReflected.y, globalReflected.z })};

	//return Ray{ intersectPoint, globalReflected };
	return Ray{ intersectPoint, glm::vec4(glm::normalize(Z), 1.f) };
}

inline Direction computeShadowRayDirection(const Vertex& point, const Vertex& lightPoint)
{
	return glm::normalize(glm::vec3(lightPoint) - glm::vec3(point));
}

inline bool objectIsVisible(const Ray& ray, const SceneObject& obj, const std::optional<IntersectionData>& input, const Direction& normal)
{
	return !(
		input.has_value() // Intersection must exist
		// Check if intersection is on the right side of the light (maybe this could be improved performance-wise?)
		&& glm::length(glm::vec3(ray.getEnd() - ray.getStart())) > glm::length(input->_t * ray.getNormalizedDirection())
		&& obj.getBRDF().getSurfaceType() != BRDF::TRANSPARENT
		);
}

inline double shadowRayContribution(const Vertex& point, const Vertex& lightPoint, const Direction& normal, SceneGeometry& scene)
{
	double lightContribution = 0.f;

	bool visible = true;
	Direction shadowRayVec = glm::normalize(glm::vec3(lightPoint) - glm::vec3(point));
	float normalDotContribution = glm::dot(shadowRayVec, normal);

	if (normalDotContribution <= 0) //Angle between normal and lightvec >= 90 deg
		return 0.0;
	else
	{
		Ray shadowRay{ point, lightPoint };

		auto itTetra = scene._tetrahedons.begin();
		auto itSphere = scene._spheres.begin();

		//This loop is ugly but efficient
		while ((itTetra != scene._tetrahedons.end() || itSphere != scene._spheres.end()) && visible)
		{
			if (itTetra != scene._tetrahedons.end() && visible)
			{
				visible = objectIsVisible(shadowRay, *itTetra, itTetra->rayIntersection(shadowRay), normal);
				++itTetra;
			}
			if (itSphere != scene._spheres.end() && visible)
			{
				visible = objectIsVisible(shadowRay, *itSphere, itSphere->rayIntersection(shadowRay), normal);
				++itSphere;
			}
		}
	}
	lightContribution += normalDotContribution * visible;

	return lightContribution;
}