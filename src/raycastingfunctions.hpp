#pragma once

#include "ray.hpp"
#include "scenegeometry.hpp"
#include "config.hpp"


/************************
	  Declarations
************************/
static constexpr float PI = 3.1415f;
static constexpr float TWO_PI = 6.28318f;
static constexpr float _airIndex = 1.f;
static constexpr float _glassIndex = 1.5f;
static constexpr float _reflectionOffset = 0.01;

inline bool pathIsVisible(Ray& ray, const Direction& normal, const SceneGeometry& scene);

/************************
	Implementations
************************/
template<typename T>
T* calcIntersection(
	std::vector<T>& objects,
	Ray& ray,
	float& minT,
	std::optional<IntersectionData>& closestIntersectData,
	SceneObject* closestIntersectObject)
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
	closestIntersectObject = calcIntersection(geometry._tetrahedrons, ray, minT, closestIntersectData, closestIntersectObject);
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
inline void calcIntersection(
	const std::vector<T>& objects,
	Ray& ray,
	std::vector<IntersectionSurface>& intersections)
{
	for (size_t i{ 0 }; i < objects.size(); ++i)
	{
		objects[i].rayIntersections(ray, intersections);
	}
}

//Specialize for TriangleObj as these can only intersect once
template<>
inline void calcIntersection<TriangleObj>(
	const std::vector<TriangleObj>& objects,
	Ray& ray,
	std::vector<IntersectionSurface>& intersections)
{
	for (size_t i{ 0 }; i < objects.size(); ++i)
	{
		auto tempIntersection = objects[i].rayIntersection(ray);
		//Did intersection occur?
		if (tempIntersection.has_value())
		{
			intersections.push_back(IntersectionSurface{
				tempIntersection.value(),
				&objects[i]});
		}
	}
}

//Fills intersections with data and sorts increasingly by distance from photon origin (AKA closest first)
inline void photonIntersection(Ray& ray, const SceneGeometry& geometry, std::vector<IntersectionSurface>& intersections)
{
	calcIntersection(geometry._sceneTris, ray, intersections);
	calcIntersection(geometry._tetrahedrons, ray, intersections);
	calcIntersection(geometry._spheres, ray, intersections);

	std::sort(intersections.begin(), intersections.end(), [](IntersectionSurface& a, IntersectionSurface& b)
		{
			return a.intersectionData._t < b.intersectionData._t;
		});
}


inline Ray computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint)
{
	Direction incomingRayDirection = incomingRay.getNormalizedDirection();

	Direction reflectedDirection =
		incomingRayDirection - 2.f * (glm::dot(incomingRayDirection, normal)) * normal;

	glm::vec4 offset = glm::vec4(reflectedDirection * _reflectionOffset, 0);

	return Ray{ Vertex{ intersectionPoint + offset },
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

	glm::vec4 offset = glm::vec4((refractDir * _reflectionOffset), 0);
	Ray result = Ray{ intersectionPoint + offset, Vertex{ glm::vec3{ intersectionPoint } + refractDir, 1.0f } };
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

	float brewsterAngle = asin(n2 / n1); // In radians
	if (currentRay.isInsideObject() && incAngle > brewsterAngle) // Total internal reflection
	{
		reflectionCoeff = 1.f;
		return false;
	}
	else
	{
		double R0 = pow((n1 - n2) / (n1 + n2), 2);
		reflectionCoeff = std::min(R0 + (1 - R0) * pow(1.0 - cos(incAngle), 5), 0.99999);
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
	float rand1, float rand2)
{

	//Determine local coordinate system and transformations matrices for it
	const glm::vec3 Z{ normal };
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

	// Generate random azimuth (phi) and inclination (theta)
	// Phi is caled to compensate for decreased range of rand1 since the ray is terminated 
	// russian roulette for high values for rand1
	const float phi = (glm::two_pi<float>() / (1 - Config::monteCarloTerminationProbability())) * rand1;
	const float theta = glm::asin(glm::sqrt(rand2));
	const float x = glm::cos(phi) * glm::sin(theta);
	const float y = glm::sin(phi) * glm::sin(theta);
	const float z = glm::cos(theta);

	const glm::vec4 globalReflected = toGlobalCoord * glm::vec4{ glm::normalize(glm::vec3(x, y, z)), 1.f };

	//Debug helper
	//const glm::vec3 globalDirection{ glm::normalize(glm::vec3{globalReflected.x, globalReflected.y, globalReflected.z })};

	glm::vec4 offset = glm::vec4(normal * _reflectionOffset, 0);
	return Ray{ intersectPoint + offset, globalReflected };
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
	Direction shadowRayVec = glm::normalize(glm::vec3(lightPoint) - glm::vec3(point));
	float normalDotContribution = glm::dot(shadowRayVec, normal);

	if (normalDotContribution <= 0) //Angle between normal and lightvec >= 90 deg
		return 0.0;
	else
	{
		Ray shadowRay{ point, lightPoint };
		return normalDotContribution * pathIsVisible(shadowRay, normal, scene);
	}
}

inline bool pathIsVisible(Ray& ray, const Direction& normal, const SceneGeometry& scene)
{
	bool visible = true;

	auto itTetra = scene._tetrahedrons.begin();
	auto itSphere = scene._spheres.begin();

	//This loop is ugly but efficient
	while ((itTetra != scene._tetrahedrons.end() || itSphere != scene._spheres.end()) && visible)
	{
		if (itTetra != scene._tetrahedrons.end() && visible)
		{
			visible = objectIsVisible(ray, *itTetra, itTetra->rayIntersection(ray), normal);
			++itTetra;
		}
		if (itSphere != scene._spheres.end() && visible)
		{
			visible = objectIsVisible(ray, *itSphere, itSphere->rayIntersection(ray), normal);
			++itSphere;
		}
	}

	return visible;
}

inline Color localAreaLightContribution(const Ray& inc, const Vertex& point,
	const Direction& normal, const SceneObject* obj, const SceneGeometry& scene)
{
	static auto _gen = std::mt19937{ std::random_device{}() };
	static auto _rng = std::uniform_real_distribution<float>{ 0.f, 1.f };

	// TODO Adapt for varying amout of lights
	auto light = scene._ceilingLights[0];

	double acc = 0;

	for (size_t i = 0; i < static_cast<size_t>(Config::numShadowRaysPerIntersection()); i++)
	{
		float rand1 = _rng(_gen);
		float rand2 = _rng(_gen);
		// Define local coord.system at light surface
		glm::vec3 v1 = light.leftFar - light.leftClose;
		glm::vec3 v2 = light.rightClose - light.leftClose;
		// Transform to global
		glm::vec3 randPointAtLight = glm::vec3(light.leftClose) + rand1 * v1 + rand2 * v2;

		glm::vec4 offset = glm::vec4(normal * _reflectionOffset, 0);
		Ray shadowRay{ point + offset, Vertex{ randPointAtLight, 1.0f } };

		if (pathIsVisible(shadowRay, normal, scene))
		{
			double lightDistance = glm::length(randPointAtLight - glm::vec3(point));
			double cosAlpha = glm::dot(-shadowRay.getNormalizedDirection(), light.getNormal());
			double cosBeta = glm::dot(shadowRay.getNormalizedDirection(), normal);

			double brdf = obj->accessBRDF().computeBRDF(
				shadowRay.getNormalizedDirection(),
				-inc.getNormalizedDirection(),
				normal);

			if (lightDistance == 0)
				std::cout << "panikorkester\n";

			acc += brdf * glm::clamp(cosAlpha * cosBeta, 0.0, 1.0) / (lightDistance * lightDistance);
		}
	}

	// TODO Hard coding area is ofc not great
	constexpr double lightArea = 1;

	constexpr double L0 = 1000.0 / (glm::pi<double>() * lightArea);
	Color returnValue = acc * obj->getColor() * (lightArea * L0 * (1.0 / Config::numShadowRaysPerIntersection()));

	return returnValue;
}
