#include "scene.hpp"

#include <glm/gtx/string_cast.hpp>

#include "ray.hpp"
std::vector<TriangleObj> Scene::_sceneTris;
std::vector<Tetrahedron> Scene::_tetrahedrons;
std::vector<Sphere> Scene::_spheres;
std::vector<PointLight> Scene::_pointLights;
std::vector<CeilingLight> Scene::_ceilingLights;
long long unsigned Scene::_nCalculations = 0;

std::mt19937 Scene::_gen = std::mt19937{ std::random_device{}() };
std::uniform_real_distribution<float> Scene::_rng = std::uniform_real_distribution<float>{ 0.f, 1.f };

Scene::Scene()
{
	_sceneTris.reserve(24);

	//Floor triangles
	for (size_t i = 0; i < floorVertices.size(); i += 3)
	{
		_sceneTris.emplace_back(
			BRDF::DIFFUSE,
			floorVertices[i], floorVertices[i + 1], floorVertices[i + 2],
			Direction{ 0.f, 0.f, 1.f },
			Color{ 1.0, 1.0, 1.0 });
	}

	//Ceiling triangles
	for (size_t i = 0; i < ceilingVertices.size(); i += 3)
	{
		_sceneTris.emplace_back(
			BRDF::DIFFUSE,
			ceilingVertices[i], ceilingVertices[i + 1], ceilingVertices[i + 2],
			Direction{ 0.f, 0.f, -1.f },
			Color{ 1.0, 1.0, 1.0 });
	}

	//// Clown room :))
	//constexpr Color wallColors[] = {
	//	Color{ 1.0, 1.0, 1.0 },
	//	Color{ 0.0, 0.7, 0.0 },
	//	Color{ 0.0, 0.0, 0.7 },
	//	Color{ 0.5, 0.5, 0.0 },
	//	Color{ 0.0, 0.5, 0.5 },
	//	Color{ 0.5, 0.0, 0.5 },
	//};

	//constexpr Color colooor = Color{ 0.2, 0.65, 0.92 };
	//constexpr Color wallColors[6] = {
	//	Color{ 0.2, 0.65, 0.92 }, Color{ 0.2, 0.65, 0.92 }, Color{ 0.2, 0.65, 0.92 },
	//	Color{ 0.54, 0.79, 0.65 }, Color{ 0.54, 0.79, 0.65 }, Color{ 0.54, 0.79, 0.65 }
	//};
	constexpr Color wallColors[6] = {
		Color{ 1, 0.2, 0.2 }, Color{ 1, 0.2, 0.2 }, Color{ 1, 0.2, 0.2 },
		Color{ 0.2, 1, 0.2 }, Color{ 0.2, 1, 0.2 }, Color{ 0.2, 1, 0.2 }
	};

	//Wall triangles
	size_t wallNormalCounter = 0;
	for (size_t i = 0; i < wallVertices.size(); i += 3)
	{
		//Since there are a lot more vertices than normals
		//Some care has to be taken when reading normals
		if (i % 6 == 0 && i != 0)
			wallNormalCounter++;

		_sceneTris.emplace_back(
			BRDF::DIFFUSE,
			wallVertices[i], wallVertices[i + 1], wallVertices[i + 2],
			glm::normalize(wallNormals[wallNormalCounter]), // The normalize is needed with the current values in wallNormals
			wallColors[(i / 3) / 2]);

	}

	//Ceiling light
	_ceilingLights.emplace_back(BRDF{ BRDF::LIGHT }, 7.f, 0.f); // These are not implemented properly yet

	//// Algots scene
	//_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 0.8f, Color{ 0.0, 0.0, 1.0 }, Vertex{ 6.0f, -1.5f, -3.0f, 1.0f });
	//_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 2.f, Color{ 0.1, 0.1, 0.1 }, Vertex{ 9.f, 0.f, -2.5f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 5.f, 0.f, -3.f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.5f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 6.f, 3.5f, -3.f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.5f, Color{ 0.1, 0.1, 0.1 }, Vertex{ 6.f, 2.5f, -3.f, 1.f });
	_spheres.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.5f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 6.f, -2.5f, -3.f, 1.f });
	//_pointLights.emplace_back(Vertex(3, 0, 4, 1), Color(1, 1, 1));
	//_pointLights.emplace_back(Vertex(0, 0, 1, 1), Color(1, 1, 1));
	//_pointLights.emplace_back(Vertex(2, 1, 4, 1), Color(1, 1, 1));
}

Color Scene::raycastScene(Ray& initialRay)
{
	RayTree tree{ initialRay };
	tree.raytracePixel(false);
	return tree.getPixelColor();
}

bool Scene::rayIntersection(Ray& ray)
{
	std::optional<IntersectionData> closestIntersectData{};
	SceneObject* closestIntersectObject = nullptr;
	float minT = 1e+10;

	closestIntersectObject = calcIntersections(_sceneTris, ray, minT, closestIntersectData, closestIntersectObject);
	closestIntersectObject = calcIntersections(_tetrahedrons, ray, minT, closestIntersectData, closestIntersectObject);
	closestIntersectObject = calcIntersections(_spheres, ray, minT, closestIntersectData, closestIntersectObject);
	closestIntersectObject = calcIntersections(_ceilingLights, ray, minT, closestIntersectData, closestIntersectObject);

	if (closestIntersectData.has_value())
	{
		ray.setIntersectedObject(closestIntersectObject);
		ray.setIntersectionData(closestIntersectData.value());
		return true;
	}
	return false;
}

template<typename T>
T* Scene::calcIntersections(std::vector<T>& container, Ray& ray, float& minT,
	std::optional<IntersectionData>& closestIntersectData, SceneObject* closestIntersectObject)
{
	T* intersectObject = static_cast<T*>(closestIntersectObject);

	for (size_t i{ 0 }; i < container.size(); ++i)
	{
		auto tempIntersection = container[i].rayIntersection(ray);
		++_nCalculations;
		//Did intersection occur, and is it closer than minT?
		if (tempIntersection.has_value() && tempIntersection.value()._t < minT)
		{
			closestIntersectData = tempIntersection;
			intersectObject = &container[i];
			minT = tempIntersection.value()._t;
		}
	}
	return intersectObject;
}

Color Scene::localAreaLightContribution(const Ray& inc, const Vertex& point, 
                                        const Direction& normal, const SceneObject* obj)
{
	// TODO Adapt for varying amout of lights
	auto light = _ceilingLights[0];

	double acc = 0;

	for (size_t i = 0; i < _numShadowRaysPerIntersection; i++)
	{
		float rand1 = _rng(_gen);
		float rand2 = _rng(_gen);
		// Define local coord.system at light surface
		glm::vec3 v1 = light.leftFar - light.leftClose;
		glm::vec3 v2 = light.rightClose - light.leftClose;
		// Transform to global
		glm::vec3 randPointAtLight = glm::vec3(light.leftClose) + rand1 * v1 + rand2 * v2;
		//std::cout << std::string(glm::to_string(randPointAtLight) + '\n');

		glm::vec4 offset = glm::vec4(normal * _reflectionOffset, 0);
		Ray shadowRay{ point + offset, Vertex{ randPointAtLight, 1.0f } };

		if (pathIsVisible(shadowRay, normal))
		{
			double lightDistance = glm::length(randPointAtLight - glm::vec3(point));
			double cosAlpha = glm::dot(-shadowRay.getNormalizedDirection(), light.getNormal());
			double cosBeta = glm::dot(shadowRay.getNormalizedDirection(), normal);

			double brdf = obj->accessBRDF().computeOrenNayar(
				shadowRay.getNormalizedDirection(),
				-inc.getNormalizedDirection(),
				normal);

			acc += brdf * cosAlpha * cosBeta / (lightDistance * lightDistance);
			//acc += 1;
		}
	}

	// TODO Hard coding area is ofc terrible
	double lightArea = 1;
	// TODO Is it correct that L0 is the color of the light?
	Color L0 = light.getColor();
	//return Color(acc);
	return acc * obj->getColor() * (lightArea * L0 * (1.0 / _numShadowRaysPerIntersection));
}

bool Scene::pathIsVisible(Ray& ray, const Direction& normal)
{
	bool visible = true;

	auto itTetra = _tetrahedrons.begin();
	auto itSphere = _spheres.begin();

	//This loop is ugly but efficient
	while ((itTetra != _tetrahedrons.end() || itSphere != _spheres.end()) && visible)
	{
		if (itTetra != _tetrahedrons.end() && visible)
		{
			visible = objectIsVisible(ray, *itTetra, itTetra->rayIntersection(ray), normal);
			++itTetra;
		}
		if (itSphere != _spheres.end() && visible)
		{
			visible = objectIsVisible(ray, *itSphere, itSphere->rayIntersection(ray), normal);
			++itSphere;
		}
	}

	return visible;
}

double Scene::shadowRayContribution(const Vertex& point, const Direction& normal)
{
	double lightContribution = 0.f;

	for (const auto& light : _pointLights)
	{
		Direction shadowRayVec = glm::normalize(glm::vec3(light.getPosition()) - glm::vec3(point));
		float normalDotContribution = glm::dot(shadowRayVec, normal);

		if (normalDotContribution <= 0) //Angle between normal and lightvec >= 90 deg
			continue;

		Ray shadowRay{ point, light.getPosition() };

		lightContribution += normalDotContribution * pathIsVisible(shadowRay, normal);
	}
	return lightContribution;
}

bool Scene::objectIsVisible(const Ray& ray, const SceneObject& obj, const std::optional<IntersectionData>& input, const Direction& normal)
{
	return !(
		input.has_value() // Intersection must exist
		// Check if intersection is on the right side of the light (maybe this could be improved performance-wise?)
		&& glm::length(glm::vec3(ray.getEnd() - ray.getStart())) > glm::length(input->_t * ray.getNormalizedDirection())
		&& obj.getBRDF().getSurfaceType() != BRDF::TRANSPARENT
		);
}

Ray Scene::computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject)
{
	Direction incomingRayDirection = incomingRay.getNormalizedDirection();
	//Angle between normal and incoming ray
	float angle = glm::angle(normal, incomingRayDirection);

	Direction reflectedDirection =
		incomingRayDirection - 2.f * (glm::dot(incomingRayDirection, normal)) * normal;

	glm::vec4 offset = glm::vec4(normal * _reflectionOffset, 1);

	return Ray{ Vertex{ intersectionPoint + offset },
				Vertex{ Direction(intersectionPoint) + reflectedDirection, 1.f } };
}

Ray Scene::computeRefractedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject)
{
	Direction incomingDir = incomingRay.getNormalizedDirection();
	const float n1n2 = insideObject ? _glassIndex / _airIndex : _airIndex / _glassIndex;
	const float NI = glm::dot(normal, incomingDir);
	const float sqrtExpression = 1 - ((glm::pow(n1n2, 2)) * (1 - glm::pow(NI, 2)));

	Direction refractDir = n1n2 * incomingDir + normal * (-n1n2 * NI
		- glm::sqrt(sqrtExpression)
		);

	glm::vec4 offset = glm::vec4(-1.0f * (normal * _reflectionOffset), 0);

	return Ray{ intersectionPoint + offset, Vertex{ glm::vec3{ intersectionPoint } + refractDir, 1.0f } };
}

Scene::RayTree::RayTree(Ray& initialRay)
	: _gen{ std::random_device{}() }, _rng{ 0.f, 1.f }
{
	_head = std::make_unique<Ray>(initialRay);
	_treeSize = 1;
}

void Scene::RayTree::raytracePixel(bool isMonteCarloTree)
{
	constructRayTree(isMonteCarloTree);

	_finalColor = traverseRayTree(_head.get(), isMonteCarloTree);
}

Direction Scene::computeShadowRayDirection(const Vertex& point)
{
	return glm::normalize(glm::vec3(_pointLights[0].getPosition()) - glm::vec3(point));
}

//void Scene::RayTree::monteCarloDiffuseContribution(Ray* initialRay, const IntersectionData& initialIntersection, const SceneObject* intersectObj)
//{
//	Ray firstRandomReflectedRay = generateRandomReflectedRay(initialRay->getNormalizedDirection(), initialIntersection._normal, initialIntersection._intersectPoint);
//	firstRandomReflectedRay.setParent(initialRay);
//	firstRandomReflectedRay.setColor(initialRay->getColor() * intersectObj->getColor());
//
//	RayTree monteCarloTree{ firstRandomReflectedRay };
//	//monteCarloTree.raytracePixel(true);
//	monteCarloTree.constructRayTree(true);
//
//	initialRay->setLeft(std::move(*monteCarloTree._head));
//	//initialRay->setColor(monteCarloTree.getPixelColor());
//}

Ray Scene::RayTree::generateRandomReflectedRay(const Direction& initialDirection, const Direction& normal, const Vertex& intersectPoint, float rand1, float rand2)
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

	//Generate random azimuth (phi) and inclination (theta)
	// Scaled to compensate for the russian roulette termination
	const float phi = (glm::two_pi<float>() / (1 - _terminationProbability)) * rand1;
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

void Scene::RayTree::constructRayTree(bool isMonteCarloTree)
{
	//TODO this method needs to be shortened
	std::queue<Ray*> rays;
	Ray* currentRay = _head.get();
	rays.push(currentRay);

	size_t rayTreeCounter{ 0 };
	while (!rays.empty() && rayTreeCounter < _maxTreeSize)
	{
		currentRay = rays.front();
		rays.pop();

		auto rayImportance = currentRay->getColor();

		// This should NOT be used with monte carlo
		// Ray will not contribute much to final image
		//static constexpr float EPS = 0.0001;
		//if (rayImportance.r < EPS && rayImportance.g < EPS && rayImportance.b < EPS)			
		//	continue;

		// The rayIntersection method adds intersection info to ray
		bool intersected = rayIntersection(*currentRay);
		if (!intersected)
		{
			std::cout << "A ray with no intersections detected\n";
			continue;
		}

		const auto& currentIntersection = currentRay->getIntersectionData().value();
		const auto& currentIntersectObject = currentRay->getIntersectedObject().value();
		const auto& currentSurfaceType = currentIntersectObject->getBRDF().getSurfaceType();

		if (currentSurfaceType == BRDF::LIGHT) // Terminate on light
			;/*currentRay->setColor(Color(1.0, 1.0, 1.0));*/
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
			float rand1 = _rng(_gen);
			float rand2 = _rng(_gen);
			if (rand1 + _terminationProbability > 1.f) //Terminate ray
				//currentRay->setColor(currentRay->getColor() * (1.0 / _terminationProbability));
				;
			else
			{
				attachReflectedMonteCarlo(currentIntersection, currentRay, rand1, rand2);

				const double roughness = currentIntersectObject->accessBRDF().computeOrenNayar(
					currentRay->getLeft()->getNormalizedDirection(),
					-currentRay->getNormalizedDirection(),
					currentIntersection._normal);



				//if (roughness > 1)
				//{
				//	std::cout << "Jas�\n";
				//}



				currentRay->getLeft()->setColor(
					(glm::pi<double>() / (1.0 - _terminationProbability))
					* currentRay->getColor()
					* currentIntersectObject->getColor()
					* roughness);




				//if (
				//	   (currentRay->getLeft()->getColor().r > currentRay->getColor().r)
				//	|| (currentRay->getLeft()->getColor().g > currentRay->getColor().g)
				//	|| (currentRay->getLeft()->getColor().b > currentRay->getColor().b)
				//   )
				//{
				//	std::cout << "NOOoooooooOooO\n";
				//}




				rays.push(currentRay->getLeft());
				++rayTreeCounter;
			}
		}
		else if (currentSurfaceType == BRDF::TRANSPARENT)
		{


			//// TEST
			//static constexpr float EPS = 0.0001;
			//if (rayImportance.r < EPS && rayImportance.g < EPS && rayImportance.b < EPS)			
			//	continue;



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
			{
				double R0 = pow((n1 - n2) / (n1 + n2), 2);
				reflectionCoeff = R0 + (1 - R0) * pow(1.0 - cos(incAngle), 5);

				attachRefracted(currentIntersection, currentRay);

				currentRay->getRight()->setColor((1.0 - reflectionCoeff) * currentRay->getColor());
				rays.push(currentRay->getRight());
				++rayTreeCounter;

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

Color Scene::RayTree::traverseRayTree(Ray* input, bool isMonteCarloTree) const
{
	Ray* currentRay = input;

	Ray* left = currentRay->getLeft();
	Ray* right = currentRay->getRight();

	// TODO Local lighting model contibutions

	// TODO Ideally the ray should always intersect something
	if (currentRay->getIntersectionData().has_value() == false)
		return Color{ 0 };
		//return Color{ 0,0,1 };
	//auto surfaceType = currentRay->getIntersectedObject().value()->getBRDF();
	auto& intersectData = currentRay->getIntersectionData().value();
	auto& intersectObject = currentRay->getIntersectedObject().value();

	Color localLightContribution = Color{ 0 };
	if (intersectObject->getBRDF().getSurfaceType() != BRDF::TRANSPARENT)
	{
		localLightContribution = localAreaLightContribution(
			*currentRay,
			intersectData._intersectPoint,
			intersectData._normal,
			intersectObject);
	}

	if (left == nullptr && right == nullptr)
	{
		//auto returnValue = currentRay->getColor() * intersectObject->getColor()
		//	* shadowRayContribution(intersectData._intersectPoint,
		//		intersectData._normal);

		//if (glm::length(localLightContribution) > 0.001)
		//{
		//	std::cout << "hkhjkl\n";
		//}

		//if (intersectObject->getBRDF().getSurfaceType() == BRDF::LIGHT)
		//	return intersectObject->getColor();
		//else
			return localLightContribution;
		//return ;
	}
	else if (left && right == nullptr)
	{
		//if (
		//	(left->getColor() / currentRay->getColor()).r > 1
		//	|| (left->getColor() / currentRay->getColor()).g > 1
		//	|| (left->getColor() / currentRay->getColor()).b > 1
		//   )
		//{
		//	std::cout << "NOOoooooooOooO\n";
		//}

		return (left->getColor() / currentRay->getColor())
			* traverseRayTree(left, isMonteCarloTree) + localLightContribution;
	}
	else if (left == nullptr && right)
	{
		return (right->getColor() / currentRay->getColor())
			* traverseRayTree(right, isMonteCarloTree) + localLightContribution;
	}
	else if (left && right)
	{
		Color leftSubContrib = traverseRayTree(currentRay->getLeft(), isMonteCarloTree) * currentRay->getLeft()->getColor();
		Color rightSubContrib = traverseRayTree(currentRay->getRight(), isMonteCarloTree) * currentRay->getRight()->getColor();

		return (leftSubContrib + rightSubContrib) / currentRay->getColor() + localLightContribution;
		//return (leftSubContrib + rightSubContrib);
	}
}

void Scene::RayTree::attachReflected(const IntersectionData& intData, Ray* currentRay) const
{
	Ray reflectedRay = Scene::computeReflectedRay(
		intData._normal,
		*currentRay,
		intData._intersectPoint,
		currentRay->isInsideObject());

	// Reflected ray will always continue in the same medium
	reflectedRay.setInsideObject(currentRay->isInsideObject());

	currentRay->setLeft(std::move(reflectedRay));
	currentRay->getLeft()->setParent(currentRay);
}

void Scene::RayTree::attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay, float rand1, float rand2)
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

void Scene::RayTree::attachRefracted(const IntersectionData& intData, Ray* currentRay) const
{
	Ray refractedRay = Scene::computeRefractedRay(
		intData._normal,
		*currentRay,
		intData._intersectPoint,
		currentRay->isInsideObject());

	// If refracting the medium changes
	refractedRay.setInsideObject(!currentRay->isInsideObject());

	currentRay->setRight(std::move(refractedRay));
	currentRay->getRight()->setParent(currentRay);
}
