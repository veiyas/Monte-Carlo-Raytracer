#include "scene.hpp"

#include <glm/gtx/string_cast.hpp>

#include "ray.hpp"
#include "util.hpp"
#include "raycastingfunctions.hpp"

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

RayTree::RayTree(Ray& initialRay, Scene* scene)
	: _gen{ std::random_device{}() }, _rng{ 0.f, 1.f },
	  _scene{ scene }
{
	_head = std::make_unique<Ray>(initialRay);
}

void RayTree::raytracePixel()
{
	constructRayTree();
	_finalColor = traverseRayTree(_head.get(), false);
}

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
				if (rand1 + Config::monteCarloTerminationProbability() > 1.f); //Terminate ray
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
					continue;
				}
			}
			else // Transmission occurs, Schlicks equation for radiance distribution
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

	auto& intersectData = currentRay->getIntersectionData().value();
	auto& intersectObject = currentRay->getIntersectedObject().value();
	auto surfaceType = intersectObject->getBRDF().getSurfaceType();

	Color localLightContribution{ 0 };

	if (surfaceType != BRDF::TRANSPARENT)
	{
		if (Config::usePhotonMapping())
		{
			bool shadowPhotonsPresent = _scene->_photonMap->areShadowPhotonsPresent(intersectData._intersectPoint);
			if (!shadowPhotonsPresent)
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
