#include "scene.hpp"

#include <glm/gtx/string_cast.hpp>

#include "ray.hpp"
#include "util.hpp"

Scene::Scene()
	: _nCalculations{ 0 },
	  _sceneGeometry{ }
{
	_gen = std::mt19937{ std::random_device{}() };
	_rng = std::uniform_real_distribution<float>{ 0.f, 1.f };

	if (Config::usePhotonMapping())
		_photonMap = std::make_unique<PhotonMap>(_sceneGeometry);

	auto lightCenter = _sceneGeometry._ceilingLights[0].getCenterPoints();
}

Color Scene::raycastScene(Ray& initialRay)
{
	RayTree tree{ initialRay, this };
	tree.raytracePixel();
	return tree.getPixelColor();
}
//
//Color Scene::localAreaLightContribution(const Ray& inc, const Vertex& point, 
//                                        const Direction& normal, const SceneObject* obj) const
//{
//	// TODO Adapt for varying amout of lights
//	auto light = _ceilingLights[0];
//
//	double acc = 0;
//
//	for (size_t i = 0; i < _config.numShadowRaysPerIntersection; i++)
//	{
//		float rand1 = _rng(_gen);
//		float rand2 = _rng(_gen);
//		// Define local coord.system at light surface
//		glm::vec3 v1 = light.leftFar - light.leftClose;
//		glm::vec3 v2 = light.rightClose - light.leftClose;
//		// Transform to global
//		glm::vec3 randPointAtLight = glm::vec3(light.leftClose) + rand1 * v1 + rand2 * v2;
//
//		glm::vec4 offset = glm::vec4(normal * _reflectionOffset, 0);
//		Ray shadowRay{ point + offset, Vertex{ randPointAtLight, 1.0f } };
//
//		if (pathIsVisible(shadowRay, normal, _sceneGeometry))
//		{
//			double lightDistance = glm::length(randPointAtLight - glm::vec3(point));
//			double cosAlpha = glm::dot(-shadowRay.getNormalizedDirection(), light.getNormal());
//			double cosBeta = glm::dot(shadowRay.getNormalizedDirection(), normal);
//
//			double brdf = obj->accessBRDF().computeBRDF(
//				shadowRay.getNormalizedDirection(),
//				-inc.getNormalizedDirection(),
//				normal);
//
//			if (lightDistance == 0)
//				std::cout << "panikorkester\n";
//
//			brdf = 1;
//			acc += brdf * glm::clamp(cosAlpha * cosBeta, 0.0, 1.0) / (lightDistance * lightDistance);
//		}
//	}
//
//	// TODO Hard coding area is ofc terrible
//	double lightArea = 1;
//	// TODO Is it correct that L0 is the color of the light?
//	Color L0 = light.getColor();
//	Color returnValue = acc * obj->getColor() * (lightArea * L0 * (1.0 / _config.numShadowRaysPerIntersection));
//
//	return returnValue;
//}

RayTree::RayTree(Ray& initialRay, Scene* scene)
	: _gen{ std::random_device{}() }, _rng{ 0.f, 1.f },
	  _scene{ scene }
{
	_head = std::make_unique<Ray>(initialRay);
	_treeSize = 1;
}

void RayTree::raytracePixel()
{
	constructRayTree();
	_finalColor = traverseRayTree(_head.get(), false);
}

//// TODO Replace with raycastingfunctions
//Ray RayTree::generateRandomReflectedRay(const Direction& initialDirection, const Direction& normal, const Vertex& intersectPoint, float rand1, float rand2)
//{
//
//	//Determine local coordinate system and transformations matrices for it
//	const glm::vec3 Z{ normal };
//	const glm::vec3 X = glm::normalize(initialDirection - glm::dot(initialDirection, Z) * Z);
//	const glm::vec3 Y = glm::cross(-X, Z);
//	/* GLM: Column major order
//	 0  4  8  12
//	 1  5  9  13
//	 2  6 10  14
//	 3  7 11  15
//	*/
//	const glm::mat4 toLocalCoord =
//		glm::mat4{
//		X.x, Y.x, Z.x, 0.f,
//		X.y, Y.y, Z.y, 0.f,
//		X.z, Y.z, Z.z, 0.f,
//		0.f, 0.f, 0.f, 1.f } *
//		glm::mat4{
//		1.f, 0.f, 0.f, 0.f,
//		0.f, 1.f, 0.f, 0.f,
//		0.f, 0.f, 1.f, 0.f,
//		-intersectPoint.x, -intersectPoint.y, -intersectPoint.z, 1.f
//	};
//	const glm::mat4 toGlobalCoord = glm::inverse(toLocalCoord);
//
//	// Generate random azimuth (phi) and inclination (theta)
//	// Phi is caled to compensate for decreased range of rand1 since the ray is terminated 
//	// russian roulette for high values for rand1
//	const float phi = (glm::two_pi<float>() / (1 - _config.monteCarloTerminationProbability)) * rand1;
//	const float theta = glm::asin(glm::sqrt(rand2));
//	const float x = glm::cos(phi) * glm::sin(theta);
//	const float y = glm::sin(phi) * glm::sin(theta);
//	const float z = glm::cos(theta);
//
//	const glm::vec4 globalReflected = toGlobalCoord * glm::vec4{ glm::normalize(glm::vec3(x, y, z)), 1.f };
//
//	//Debug helper
//	//const glm::vec3 globalDirection{ glm::normalize(glm::vec3{globalReflected.x, globalReflected.y, globalReflected.z })};
//
//	glm::vec4 offset = glm::vec4(normal * _reflectionOffset, 0);
//	return Ray{ intersectPoint + offset, globalReflected };
//}

void RayTree::constructRayTree()
{
	std::queue<Ray*> rays;
	Ray* currentRay = _head.get();
	rays.push(currentRay);

	size_t rayTreeCounter{ 0 };
	while (!rays.empty() && rayTreeCounter < _maxTreeSize)
	{
		currentRay = rays.front();
		rays.pop();

		auto rayImportance = currentRay->getColor();

		// The rayIntersection method adds intersection info to ray
		bool intersected = rayIntersection(*currentRay, _scene->_sceneGeometry);

		if (!intersected)
		{
			std::cout << "A ray with no intersections detected\n";
			continue;
		}

		const auto& currentIntersection = currentRay->getIntersectionData().value();
		const auto& currentIntersectObject = currentRay->getIntersectedObject().value();
		const auto& currentSurfaceType = currentIntersectObject->getBRDF().getSurfaceType();

		if (currentSurfaceType == BRDF::LIGHT) // Terminate on light
			; // The importance should *not* be set to white here
		else if (currentSurfaceType == BRDF::REFLECTOR)
		{
			// All importance is reflected
			attachReflected(currentIntersection, currentRay);
			currentRay->getLeft()->setColor(currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
		else if (currentSurfaceType == BRDF::DIFFUSE)
		{
			// If photon mapping is used the reflection is handled by the photon map unless in shadow
			if (!Config::usePhotonMapping() ||
				(Config::usePhotonMapping() && _scene->_photonMap->areShadowPhotonsPresent(currentIntersection._intersectPoint)))
			//if (!Config::usePhotonMapping())
			{
				float rand1 = _rng(_gen);
				float rand2 = _rng(_gen);
				if (rand1 + Config::monteCarloTerminationProbability() > 1.f) //Terminate ray
					;
				else
				{
					attachReflectedMonteCarlo(currentIntersection, currentRay, rand1, rand2);

					const double roughness = currentIntersectObject->accessBRDF().computeBRDF(
						currentRay->getLeft()->getNormalizedDirection(),
						-currentRay->getNormalizedDirection(),
						currentIntersection._normal);

					currentRay->getLeft()->setColor(
						(glm::pi<double>() / (1.0 - Config::monteCarloTerminationProbability()))
						* currentRay->getColor()
						* currentIntersectObject->getColor()
						* roughness);

					rays.push(currentRay->getLeft());
					++rayTreeCounter;
				}
			}
		}
		else if (currentSurfaceType == BRDF::TRANSPARENT)
		{
			float incAngle = glm::angle(-currentRay->getNormalizedDirection(), currentIntersection._normal);

			// How much of the incoming importance/radiance is reflected, between 0 and 1.
			// The rest of the importance/radiance is transmitted.
			double reflectionCoeff, n1, n2;

			if (currentRay->isInsideObject())
				n1 = _glassIndex, n2 = _airIndex;
			else
				n1 = _airIndex, n2 = _glassIndex;

			float brewsterAngle = asin(_airIndex / _glassIndex); // In radians // TODO Store this somewhere!

			if (currentRay->isInsideObject() && incAngle > brewsterAngle) // Total internal reflection
			{
				reflectionCoeff = 1.f;

				// Cut off ray if internally reflected more than one time
				// Left: reflected, Right: refracted
				bool isReflected = currentRay->getParent()->getLeft() == currentRay;
				if (isReflected)
				{
					//std::cout << "test!\n";
					continue;
				}
			}
			else // Transmission occurs, Schlicks equation for radiance distribution

			// TODO Incorporate these changes with the method below and clean up
//=======
//			bool rayIsTransmitted = shouldRayTransmit(n1, n2, reflectionCoeff, incAngle, *currentRay);
//
//			if(rayIsTransmitted) // Transmission occurs, Schlicks equation for radiance distribution
//>>>>>>> master
			{
				double R0 = pow((n1 - n2) / (n1 + n2), 2);
				reflectionCoeff = R0 + (1 - R0) * pow(1.0 - cos(incAngle), 5);

				attachRefracted(currentIntersection, currentRay);

				currentRay->getRight()->setColor((1.0 - reflectionCoeff) * currentRay->getColor());
				rays.push(currentRay->getRight());
				++rayTreeCounter;

				// Cut off internal reflection if transmitted from inside object
				if (currentRay->isInsideObject())
					continue;
			}

			attachReflected(currentIntersection, currentRay);
			currentRay->getLeft()->setColor(reflectionCoeff * currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
	}
}

// I leave this here for now -- the plan is to rewrite the new version to be iterative,
// and this will be a useful reference
//Color Scene::RayTree::traverseRayTree(Ray* input, bool isMonteCarloTree) const
//{
//	Ray* currentRay = input;
//	Color result{};
//
//	while (currentRay != nullptr)
//	{
//
//		Ray* left = currentRay->getLeft();
//		Ray* right = currentRay->getRight();
//
//		// TODO Local lighting model contibutions
//
//		// TODO Ideally the ray should always intersect something
//		if (currentRay->getIntersectionData().has_value() == false)
//			return result;
//		//auto surfaceType = currentRay->getIntersectedObject().value()->getBRDF();
//		auto& intersectData = currentRay->getIntersectionData().value();
//		auto& intersectObject = currentRay->getIntersectedObject().value();
//		//Color localLightContribution = 
//		//	currentRay->getColor()
//		//	* intersectObject->getColor()
//		//	* shadowRayContribution(intersectData._intersectPoint, intersectData._normal);
//		Color localLightContribution = /*currentRay->getColor() */ localAreaLightContribution(
//			*currentRay,
//			intersectData._intersectPoint,
//			intersectData._normal,
//			intersectObject);
//
//		if (left == nullptr && right == nullptr)
//		{
//			//auto returnValue = currentRay->getColor() * intersectObject->getColor()
//			//	* shadowRayContribution(intersectData._intersectPoint,
//			//		intersectData._normal);
//			return localLightContribution;
//			//return ;
//		}
//		else if (left && right == nullptr)
//		{
//			//result += localLightContribution;
//			currentRay = left;
//		}
//		else if (left == nullptr && right)
//		{
//			//result += localLightContribution;
//			currentRay = right;
//		}
//		else if (left && right)
//		{
//			Color leftSubContrib = traverseRayTree(currentRay->getLeft(), isMonteCarloTree) * currentRay->getLeft()->getColor();
//			Color rightSubContrib = traverseRayTree(currentRay->getRight(), isMonteCarloTree) * currentRay->getRight()->getColor();
//
//			result += (leftSubContrib + rightSubContrib) / currentRay->getColor();
//			//return (leftSubContrib + rightSubContrib);
//		}
//	}
//	return result;
//}


// THIS IS COMPLETELY RECURSIVE FOR NOW; i cant be bothered to figure out any other 
// way atm
Color RayTree::traverseRayTree(Ray* input, bool hasBeenDiffuselyReflected) const
{
	Ray* currentRay = input;

	// DEBUG
	if (someComponent(currentRay->getColor(), static_cast<bool(*)(double)>(std::isnan)))
		std::cout << "isnan\n";

	Ray* left = currentRay->getLeft();
	Ray* right = currentRay->getRight();

	// Ideally the ray should always intersect something
	if (currentRay->getIntersectionData().has_value() == false)
		return Color{ 0 };

	//auto surfaceType = currentRay->getIntersectedObject().value()->getBRDF();
	auto& intersectData = currentRay->getIntersectionData().value();
	auto& intersectObject = currentRay->getIntersectedObject().value();
	auto surfaceType = intersectObject->getBRDF().getSurfaceType();

	Color localLightContribution{ 0 };

	//// TESTING: Visualizing photon map, dont remove plz ==============================
	//bool shadowPhotonsPresent = _scene->_photonMap->areShadowPhotonsPresent(intersectData._intersectPoint);
	//if (!shadowPhotonsPresent)
	//{
	//	Color photonContrib = _scene->_photonMap->getPhotonRadianceContrib(
	//		-currentRay->getNormalizedDirection(), intersectObject, intersectData);

	//	return photonContrib;
	//}
	//else return 1.0 * Color{ 1,0,1 };
	//// ================================================================================

	if (surfaceType != BRDF::TRANSPARENT)
	{
		if (Config::usePhotonMapping())
		{
			bool shadowPhotonsPresent = _scene->_photonMap->areShadowPhotonsPresent(intersectData._intersectPoint);
			if (!shadowPhotonsPresent)
			//{
			//if (!left)
			{
				localLightContribution = _scene->_photonMap->getPhotonRadianceContrib(
					-currentRay->getNormalizedDirection(), intersectObject, intersectData);
			}
			else
			{
				localLightContribution = localAreaLightContribution(
					*currentRay,
					intersectData._intersectPoint,
					intersectData._normal,
					intersectObject,
					_scene->_sceneGeometry);
			}
		}
		else
		{
			localLightContribution = localAreaLightContribution(
				*currentRay,
				intersectData._intersectPoint,
				intersectData._normal,
				intersectObject,
				_scene->_sceneGeometry);
		}
	}

	if (left == nullptr && right == nullptr)
	{
		if (surfaceType == BRDF::LIGHT && !hasBeenDiffuselyReflected)
			return Color{ 1000.0 / glm::pi<double>() };
		else
			return localLightContribution;
	}
	else if (left && right == nullptr)
	{
		if (surfaceType == BRDF::DIFFUSE)
			hasBeenDiffuselyReflected = true;

		return safeDivide(left->getColor(), currentRay->getColor()) *
			traverseRayTree(left, hasBeenDiffuselyReflected) + localLightContribution;
	}
	else if (left == nullptr && right)
	{
		return safeDivide(right->getColor(), currentRay->getColor()) *
			traverseRayTree(right, hasBeenDiffuselyReflected) + localLightContribution;
	}
	else if (left && right) // Only happens for transparent objects
	{
		Color leftSubContrib = traverseRayTree(currentRay->getLeft(), hasBeenDiffuselyReflected) * left->getColor();
		Color rightSubContrib = traverseRayTree(currentRay->getRight(), hasBeenDiffuselyReflected) * right->getColor();

		return safeDivide((leftSubContrib + rightSubContrib), currentRay->getColor()) + localLightContribution;
	}
}

void RayTree::attachReflected(const IntersectionData& intData, Ray* currentRay) const
{
	Ray reflectedRay = computeReflectedRay(
		intData._normal,
		*currentRay,
		intData._intersectPoint);

	// Reflected ray will always continue in the same medium
	reflectedRay.setInsideObject(currentRay->isInsideObject());

	currentRay->setLeft(std::move(reflectedRay));
	currentRay->getLeft()->setParent(currentRay);
}

void RayTree::attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay, float rand1, float rand2)
{
	Ray reflectedRay = generateRandomReflectedRay(
		currentRay->getNormalizedDirection(),
		intData._normal,
		intData._intersectPoint,
		rand1, rand2);
	reflectedRay.setInsideObject(currentRay->isInsideObject()); // TODO Is this ever true?

	currentRay->setLeft(std::move(reflectedRay));
	currentRay->getLeft()->setParent(currentRay);
}

void RayTree::attachRefracted(const IntersectionData& intData, Ray* currentRay) const
{
	Ray refractedRay = computeRefractedRay(
		intData._normal,
		*currentRay,
		intData._intersectPoint,
		currentRay->isInsideObject());

	currentRay->setRight(std::move(refractedRay));
	currentRay->getRight()->setParent(currentRay);
}
