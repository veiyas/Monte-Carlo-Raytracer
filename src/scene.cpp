#include "scene.hpp"
long long unsigned Scene::_nCalculations = 0;

Scene::Scene()
	:_sceneGeometry{}, _photonMap{ _sceneGeometry }
{
	auto lightCenter = _sceneGeometry._ceilingLights[0].getCenterPoints();
	_pointLight = Vertex{lightCenter.first, lightCenter.second, 5.5f, 1.f};
}

Color Scene::raycastScene(Ray& initialRay)
{
	RayTree tree{ initialRay, this };
	tree.raytracePixel(false);
	return tree.getPixelColor();
}

RayTree::RayTree(Ray& initialRay, Scene* scene)
	: _gen{ std::random_device{}()}, _rng{0.f, 1.f}, _scene{scene}
{
	_head = std::make_unique<Ray>(initialRay);
	_treeSize = 1;
}

void RayTree::raytracePixel(bool isMonteCarloTree)
{
	constructRayTree(isMonteCarloTree);
	_finalColor = traverseRayTree(_head.get(), isMonteCarloTree);
}

void RayTree::monteCarloDiffuseContribution(
	Ray* initialRay,
	const IntersectionData& initialIntersection,
	const SceneObject* intersectObj)
{
	Ray firstRandomReflectedRay = generateRandomReflectedRay(
		initialRay->getNormalizedDirection(),
		initialIntersection._normal,
		initialIntersection._intersectPoint,
		_gen,
		_rng);
	firstRandomReflectedRay.setParent(initialRay);
	firstRandomReflectedRay.setColor(initialRay->getColor() * intersectObj->getColor());

	RayTree monteCarloTree{ firstRandomReflectedRay, _scene };
	monteCarloTree.raytracePixel(true);
	
	initialRay->setLeft(std::move(*monteCarloTree._head));
	initialRay->setColor(monteCarloTree.getPixelColor());
}

void RayTree::constructRayTree(const bool& isMonteCarloTree)
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

		// Ray will not contribute much to final image
		static constexpr float EPS = 0.0001;
		if (rayImportance.r < EPS && rayImportance.g < EPS && rayImportance.b < EPS)			
			continue;

		// The rayIntersection method adds intersection info to ray
		bool intersected = rayIntersection(*currentRay, _scene->_sceneGeometry);
		if (!intersected)
		{
			//std::cout << "A ray with no intersections detected\n";
			continue;
		}

		const auto& currentIntersection = currentRay->getIntersectionData().value();
		const auto& currentIntersectObject = currentRay->getIntersectedObject().value();
		const auto& currentSurfaceType = currentIntersectObject->getBRDF().getSurfaceType();

		if (currentSurfaceType == BRDF::LIGHT)
			currentRay->setColor(Color(1.0, 1.0, 1.0));
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
				double roughness = currentIntersectObject->accessBRDF().computeOrenNayar(
					currentRay->getNormalizedDirection(),
					computeShadowRayDirection(currentIntersection._intersectPoint, _scene->_pointLight),
					currentIntersection._normal);
				double photonContrib = _scene->_photonMap.getPhotonFlux(currentIntersection._intersectPoint);				
				currentRay->setColor(currentIntersectObject->getColor() * photonContrib * roughness);
			//bool shadowPhotonsPresent = _scene->_photonMap.areShadowPhotonsPresent(currentIntersection._intersectPoint);
			//if (!shadowPhotonsPresent)
			//{
			//}
			//else
			//{
			//	if (isMonteCarloTree)
			//	{
			//		float terminator = _rng(_gen);
			//		if (terminator + _terminationProbability > 1.f) //Terminate ray
			//			currentRay->setColor(currentRay->getColor() * (1.0 / _terminationProbability));
			//		else
			//		{
			//			attachReflectedMonteCarlo(currentIntersection, currentRay);
			//			const double roughness = currentIntersectObject->accessBRDF().computeOrenNayar(
			//				currentRay->getLeft()->getNormalizedDirection(),
			//				computeShadowRayDirection(currentIntersection._intersectPoint, _scene->_pointLight),
			//				currentIntersection._normal);

			//			const double shadowRayContrib = shadowRayContribution(
			//				currentIntersection._intersectPoint,
			//				_scene->_pointLight,
			//				currentIntersection._normal,
			//				_scene->_sceneGeometry
			//			);

			//			currentRay->getLeft()->setColor(
			//				currentRay->getColor()
			//				* currentIntersectObject->getColor()
			//				* roughness * shadowRayContrib);

			//			rays.push(currentRay->getLeft());
			//			++rayTreeCounter;
			//		}
			//	}
			//	else
			//		monteCarloDiffuseContribution(currentRay, currentIntersection, currentIntersectObject);
			//}
		}
		else if (currentSurfaceType == BRDF::TRANSPARENT)
		{
			float incAngle = glm::angle(-currentRay->getNormalizedDirection(), currentIntersection._normal);

			// How much of the incoming importance/radiance is reflected, between 0 and 1.
			// The rest of the importance/radiance is transmitted.
			double reflectionCoeff, n1, n2;
			bool rayIsTransmitted = shouldRayTransmit(n1, n2, reflectionCoeff, incAngle, *currentRay);
			
			if(rayIsTransmitted) // Transmission occurs, Schlicks equation for radiance distribution
			{
				double R0 = pow((n1 - n2) / (n1 + n2), 2);
				reflectionCoeff = R0 + (1 - R0) * pow(1.0 - cos(incAngle), 5);

				attachRefracted(currentIntersection, currentRay);

				currentRay->getRight()->setColor((1.0 - reflectionCoeff) * currentRay->getColor());
				rays.push(currentRay->getRight());
				++rayTreeCounter;
			}
			attachReflected(currentIntersection, currentRay);

			currentRay->getLeft()->setColor(reflectionCoeff * currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
	}
}

Color RayTree::traverseRayTree(Ray* input, bool isMonteCarloTree) const
{
	Ray* currentRay = input;
	Color result{};

	while (currentRay != nullptr)
	{
		Ray* left = currentRay->getLeft();
		Ray* right = currentRay->getRight();

		// TODO Local lighting model contibutions

		if (left == nullptr && right == nullptr)
		{
			// TODO Ideally the ray should always intersect something
			if (currentRay->getIntersectionData().has_value())
			{
				auto& intersectData = currentRay->getIntersectionData().value();
				//auto returnValue = currentRay->getColor()
				//	* shadowRayContribution(intersectData._intersectPoint,
				//		intersectData._normal)
				auto returnValue = currentRay->getColor();
				return returnValue;
			}
			else
				return Color{};
		}
		else if (left && right == nullptr)
			currentRay = currentRay->getLeft();
		else if (left == nullptr && right)
			currentRay = currentRay->getRight();
		else if (left && right)
		{
			Color leftSubContrib = traverseRayTree(currentRay->getLeft(), isMonteCarloTree) * currentRay->getLeft()->getColor();
			Color rightSubContrib = traverseRayTree(currentRay->getRight(), isMonteCarloTree) * currentRay->getRight()->getColor();

			return (leftSubContrib + rightSubContrib) / currentRay->getColor();
			//return (leftSubContrib + rightSubContrib);
		}
	}
	return result;
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

void RayTree::attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay)
{
	Ray reflectedRay = generateRandomReflectedRay(
		currentRay->getNormalizedDirection(),
		intData._normal,
		intData._intersectPoint,
		_gen,
		_rng);
	reflectedRay.setInsideObject(currentRay->isInsideObject());

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

	// If refracting the medium changes

	currentRay->setRight(std::move(refractedRay));
	currentRay->getRight()->setParent(currentRay);
}
