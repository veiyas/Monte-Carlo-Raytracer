#include "scene.hpp"

#include <glm/gtx/string_cast.hpp>

#include "ray.hpp"
std::vector<TriangleObj> Scene::_sceneTris;
std::vector<Tetrahedron> Scene::_tetrahedrons;
std::vector<Sphere> Scene::_spheres;
std::vector<PointLight> Scene::_pointLights;
long long unsigned Scene::_nCalculations = 0;

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
			Color{ 0.8, 0.8, 0.8 });
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

	constexpr Color colooor = Color{ 0.2, 0.65, 0.92 };
	constexpr Color wallColors[6] = {
		Color{ 0.2, 0.65, 0.92 }, Color{ 0.2, 0.65, 0.92 }, Color{ 0.2, 0.65, 0.92 },
		Color{ 0.54, 0.79, 0.65 }, Color{ 0.54, 0.79, 0.65 }, Color{ 0.54, 0.79, 0.65 }
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
			wallColors[(i/3)/2]);

	}

	// Tetrahedrons
	//_tetrahedrons.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.80, 0.0, 0.80 }, Vertex{ 5.0f, 0.0f, 0.f, 1.0f });
	//_tetrahedrons.emplace_back(BRDF{ BRDF::TRANSPARENT }, 0.8f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 3.0f, 0.0f, -1.0f, 1.0f });
	//_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.0f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 5.0f, -2.0f, -2.0f, 1.0f });
	// Intersecting the walls
	//_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 3.0f, Color{ 0.79, 0.0, 0.0 }, Vertex{ 13.0f, 0.0f, 0.0f, 1.0f });

	//Spheres
	//_spheres.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 6.f, 1.f, 0.f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 2.f, -2.f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 4.5f, -0.9f, 1.2f, 1.f });

	//Lights
	//_pointLights.emplace_back(Vertex(1, -1, 1, 1), Color(1, 1, 1));
	//_pointLights.emplace_back(Vertex(1, 0, 0, 1), Color(1, 1, 1)); // Centered

	//// Algots scene
	//_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 0.8f, Color{ 1.0, 0.0, 0.0 }, Vertex{ 3.0f, 2.0f, -1.0f, 1.0f });
	_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 2.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 9.f, 0.f, -2.5f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 5.f, 0.f, -3.f, 1.f });
	_spheres.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.5f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 6.f, 3.5f, -3.f, 1.f });
	_spheres.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.5f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 6.f, -3.5f, -3.f, 1.f });
	//_pointLights.emplace_back(Vertex(3, 0, 4, 1), Color(1, 1, 1));
	//_pointLights.emplace_back(Vertex(0, 0, 1, 1), Color(1, 1, 1));
	_pointLights.emplace_back(Vertex(2, 1, 4, 1), Color(1, 1, 1));
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
	SceneObject* closestIntersectObject{};
	float minT = 1e+10;

	//TODO put these in single loop
	for (auto& triangle : _sceneTris)
	{
		auto temp = triangle.rayIntersection(ray);
		++_nCalculations;
		// Did intersection occur, and is it closer than minT?
		if (temp.has_value() && temp.value()._t < minT)
		{
			closestIntersectData = temp;
			closestIntersectObject = &triangle;
			minT = temp.value()._t;
		}
	}

	for (auto& tetrahedron : _tetrahedrons)
	{
		auto temp = tetrahedron.rayIntersection(ray);
		++_nCalculations;
		if (temp.has_value() && temp.value()._t < minT)
		{
			closestIntersectData = temp;
			closestIntersectObject = &tetrahedron;
			minT = temp.value()._t;
		}
	}

	for (auto& sphere : _spheres)
	{
		auto temp = sphere.rayIntersection(ray);
		++_nCalculations;
		if (temp.has_value() && temp.value()._t < minT)
		{
			closestIntersectData = temp;
			closestIntersectObject = &sphere;
			minT = temp.value()._t;
		}
	}

	if (closestIntersectData.has_value())
	{
		ray.setIntersectedObject(closestIntersectObject);
		ray.setIntersectionData(closestIntersectData.value());
		return true;
	}
	return false;
}

double Scene::shadowRayContribution(const Vertex& point, const Direction& normal)
{
	double lightContribution = 0.f;

	for (const auto& light : _pointLights)
	{
		bool visible = true;
		Direction shadowRayVec = glm::normalize(glm::vec3(light.getPosition()) - glm::vec3(point));
		float normalDotContribution = glm::dot(shadowRayVec, normal);

		if (normalDotContribution <= 0) //Angle between normal and lightvec >= 90 deg
			continue;
		else
		{
			Ray shadowRay{ point, light.getPosition() };

			auto itTetra = _tetrahedrons.begin();
			auto itSphere = _spheres.begin();

			//This loop is ugly but efficient
			while ((itTetra != _tetrahedrons.end() || itSphere != _spheres.end()) && visible)
			{
				if (itTetra != _tetrahedrons.end() && visible)
				{
					visible = objectIsVisible(shadowRay, *itTetra, itTetra->rayIntersection(shadowRay), normal);
					++itTetra;
				}
				if (itSphere != _spheres.end() && visible)
				{
					visible = objectIsVisible(shadowRay, *itSphere, itSphere->rayIntersection(shadowRay), normal);
					++itSphere;
				}
			}			
		}
		lightContribution += normalDotContribution * visible;
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

Ray Scene::computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint)
{
	Direction incomingRayDirection = incomingRay.getNormalizedDirection();
	//Angle between normal and incoming ray
	float angle = glm::angle(normal, incomingRayDirection);

	Direction reflectedDirection =
		incomingRayDirection - 2.f * (glm::dot(incomingRayDirection, normal)) * normal;

	return Ray{ Vertex{ intersectionPoint },
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

	return Ray{ intersectionPoint, Vertex{ glm::vec3{ intersectionPoint } + refractDir, 1.0f } };
}

Scene::RayTree::RayTree(Ray& initialRay)
	: _gen{ std::random_device{}()}, _rng{0.f, 1.f}
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

void Scene::RayTree::monteCarloDiffuseContribution(Ray* initialRay, const IntersectionData& initialIntersection, const SceneObject* intersectObj)
{
	Ray firstRandomReflectedRay = generateRandomReflectedRay(initialRay->getNormalizedDirection(), initialIntersection._normal, initialIntersection._intersectPoint);
	firstRandomReflectedRay.setParent(initialRay);
	firstRandomReflectedRay.setColor(initialRay->getColor() * intersectObj->getColor());

	RayTree monteCarloTree{ firstRandomReflectedRay };
	monteCarloTree.raytracePixel(true);
	
	initialRay->setLeft(std::move(*monteCarloTree._head));
	initialRay->setColor(monteCarloTree.getPixelColor());
}

Ray Scene::RayTree::generateRandomReflectedRay(const Direction& initialDirection, const Direction& normal, const Vertex& intersectPoint)
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
	const float phi = TWO_PI * _rng(_gen);
	const float theta = glm::asin(glm::sqrt(_rng(_gen)));
	const float x = glm::cos(phi) * glm::sin(theta);
	const float y = glm::sin(phi) * glm::sin(theta);
	const float z = glm::cos(theta);

	const glm::vec4 globalReflected = toGlobalCoord * glm::vec4{ glm::normalize(glm::vec3(x, y, z)), 1.f };

	//Debug helper
	//const glm::vec3 globalDirection{ glm::normalize(glm::vec3{globalReflected.x, globalReflected.y, globalReflected.z })};

	return Ray{ intersectPoint, globalReflected };
}

void Scene::RayTree::constructRayTree(const bool& isMonteCarloTree)
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

		// Ray will not contribute much to final image
		static constexpr float EPS = 0.0001;
		if (rayImportance.r < EPS && rayImportance.g < EPS && rayImportance.b < EPS)			
			continue;

		// The rayIntersection method adds intersection info to ray
		bool intersected = rayIntersection(*currentRay);
		if (!intersected)
		{
			std::cout << "A ray with no intersections detected\n";
			continue; // I think this is right...
		}

		const auto& currentIntersection = currentRay->getIntersectionData().value();
		const auto& currentIntersectObject = currentRay->getIntersectedObject().value();

		if (currentIntersectObject->getBRDF().getSurfaceType() == BRDF::REFLECTOR)
		{
			// All importance is reflected
			attachReflected(currentIntersection, currentRay);
			currentRay->getLeft()->setColor(currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
		else if (currentIntersectObject->getBRDF().getSurfaceType() == BRDF::DIFFUSE)
		{
			if (isMonteCarloTree)
			{
				float terminator = _rng(_gen);
				if (terminator + _terminationProbability > 1.f) //Terminate ray
					currentRay->setColor(currentRay->getColor() * (1.0 / _terminationProbability));
				else
				{
					attachReflectedMonteCarlo(currentIntersection, currentRay);
					const double roughness = currentIntersectObject->accessBRDF().computeOrenNayar(
						currentRay->getLeft()->getNormalizedDirection(),
						computeShadowRayDirection(currentIntersection._intersectPoint),
						currentIntersection._normal);

					currentRay->getLeft()->setColor(
						currentRay->getColor()
						* currentIntersectObject->getColor()
						* roughness);

					rays.push(currentRay->getLeft());
					++rayTreeCounter;
				}
			}
			else
				monteCarloDiffuseContribution(currentRay, currentIntersection, currentIntersectObject);
		}
		else if (currentIntersectObject->getBRDF().getSurfaceType() == BRDF::TRANSPARENT)
		{
			float incAngle = glm::angle(-currentRay->getNormalizedDirection(), currentIntersection._normal);

			// How much of the incoming importance/radiance is reflected, between 0 and 1.
			// The rest of the importance/radiance is transmitted.
			double reflectionCoeff, n1, n2;

			if (currentRay->isInsideObject())
				n1 = _glassIndex, n2 = _airIndex;
			else
				n1 = _airIndex, n2 = _glassIndex;

			float brewsterAngle = asin(_airIndex/_glassIndex); // In radians // TODO Store this somewhere!

			if (currentRay->isInsideObject() && incAngle > brewsterAngle) // Total internal reflection
				reflectionCoeff = 1.f;
			else // Transmission occurs, Schlicks equation for radiance distribution
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

Color Scene::RayTree::traverseRayTree(Ray* input, bool isMonteCarloTree) const
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
				auto returnValue = currentRay->getColor()
					* shadowRayContribution(intersectData._intersectPoint,
						intersectData._normal);
				return returnValue;
			}
			else
			{
				return Color{};
			}
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

void Scene::RayTree::attachReflected(const IntersectionData& intData, Ray* currentRay) const
{
	Ray reflectedRay = Scene::computeReflectedRay(
		intData._normal,
		*currentRay,
		intData._intersectPoint);

	// Reflected ray will always continue in the same medium
	reflectedRay.setInsideObject(currentRay->isInsideObject());

	currentRay->setLeft(std::move(reflectedRay));
	currentRay->getLeft()->setParent(currentRay);
}

void Scene::RayTree::attachReflectedMonteCarlo(const IntersectionData& intData, Ray* currentRay)
{
	Ray reflectedRay = generateRandomReflectedRay(
		currentRay->getNormalizedDirection(),
		intData._normal,
		intData._intersectPoint);
	reflectedRay.setInsideObject(currentRay->isInsideObject());

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
