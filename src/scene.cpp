#include "scene.hpp"

#include <glm/gtx/string_cast.hpp>

#include "ray.hpp"

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
			Color{ 0.2, 0.3, 0.97 });
	}

	//Ceiling triangles
	for (size_t i = 0; i < ceilingVertices.size(); i += 3)
	{
		_sceneTris.emplace_back(
			BRDF::DIFFUSE,
			ceilingVertices[i], ceilingVertices[i + 1], ceilingVertices[i + 2],
			Direction{ 0.f, 0.f, -1.f },
			Color{ 0.96, 0.37, 0.27 });
	}

	//constexpr Color wallColors[] = {
	//	Color{ 1.0, 1.0, 1.0 },
	//	Color{ 0.0, 0.7, 0.0 },
	//	Color{ 0.0, 0.0, 0.7 },
	//	Color{ 0.5, 0.5, 0.0 },
	//	Color{ 0.0, 0.5, 0.5 },
	//	Color{ 0.5, 0.0, 0.5 },
	//};

	//constexpr Color wallColors[] = {
	//	Color{ 0.45, 0.58, 0.48 },
	//	Color{ 0.45, 0.58, 0.48 },
	//	Color{ 0.45, 0.58, 0.48 },
	//	Color{ 0.45, 0.58, 0.48 },
	//	Color{ 0.45, 0.58, 0.48 },
	//	Color{ 0.45, 0.58, 0.48 }
	//};

	constexpr Color colooor = Color{ 1,1,1 };
	constexpr Color wallColors[6] = {
		colooor, colooor, colooor,
		colooor, colooor, colooor
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
			wallNormals[wallNormalCounter],
			wallColors[(i/3)/2]);

	}

	// Tetrahedrons
	//_tetrahedrons.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.80, 0.0, 0.80 }, Vertex{ 5.0f, 0.0f, 0.f, 1.0f });
	//_tetrahedrons.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.0f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 6.0f, -3.0f, -1.0f, 1.0f });
	_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.0f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 5.0f, -2.0f, -2.0f, 1.0f });
	// Intersecting the walls
	//_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 3.0f, Color{ 0.79, 0.0, 0.0 }, Vertex{ 13.0f, 0.0f, 0.0f, 1.0f });

	//Spheres
	//_spheres.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 6.f, 1.f, 0.f, 1.f });
	_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 2.f, -2.f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 2.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 7.f, -2.f, -1.f, 1.f });

	//Lights
	_pointLights.emplace_back(Vertex(1, -1, 1, 1), Color(1, 1, 1));
	//_pointLights.emplace_back(Vertex(1, 0, 0, 1), Color(1, 1, 1)); // Centered
}

Color Scene::raycastScene(Ray& initialRay)
{
	RayTree tree{ initialRay };
	tree.raytrace(*this);
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

double Scene::shadowRayContribution(const Vertex& point, const Direction& normal) const
{
	double lightContribution = 0.f;

	for (const auto& light : _pointLights)
	{
		bool visible = true;
		Direction shadowRayVec = glm::normalize(light.getPosition() - point);
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
					visible = objectIsVisible(itTetra->rayIntersection(shadowRay), normal);
					++itTetra;
				}
				if (itSphere != _spheres.end() && visible)
				{
					visible = objectIsVisible(itSphere->rayIntersection(shadowRay), normal);
					++itSphere;
				}
			}			
		}
		lightContribution += normalDotContribution; // *visible; TODO
	}
	return lightContribution;
}

bool Scene::objectIsVisible(const std::optional<IntersectionData>& input, const Direction& normal) const
{
	if (input.has_value() // Intersection must exist
		//&& input.first < 1 // Intersections with t > 1 are behind the light
		// TODO Check for transparency
		&& normal != input.value()._normal)
		return false;
	else
		return true;
}

Ray Scene::computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint) const
{
	Direction incomingRayDirection = incomingRay.getNormalizedDirection();
	//Angle between normal and incoming ray
	float angle = glm::angle(normal, incomingRayDirection);

	Direction reflectedDirection =
		incomingRayDirection - 2.f * (glm::dot(incomingRayDirection, normal)) * normal;

	return Ray{ Vertex{ intersectionPoint },
	            Vertex{ Direction(intersectionPoint) + reflectedDirection, 1.f } };
}

Ray Scene::computeRefractedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject) const
{
	Direction incomingDir = incomingRay.getNormalizedDirection();
	float n1n2 = insideObject ? _glassIndex / _airIndex : _airIndex / _glassIndex;
	float NI = glm::dot(normal, incomingDir);
	float sqrtExpression = 1 - ((glm::pow(n1n2, 2)) * (1 - glm::pow(NI, 2)));

	Direction refractDir = n1n2 * incomingDir + normal * (-n1n2 * NI
		- glm::sqrt(sqrtExpression)
	);

	return Ray{ intersectionPoint, Vertex{ glm::vec3{ intersectionPoint } + refractDir, 1.0f } };
}

Scene::RayTree::RayTree(Ray& initialRay)
{
	_head = std::make_unique<Ray>(initialRay);
	//_head->setShadow(1.0);
	_treeSize = 1;
}

void Scene::RayTree::raytrace(Scene& scene)
{
	constructRayTree(scene);
	_finalColor = traverseRayTree(scene, _head.get());
}

void Scene::RayTree::constructRayTree(Scene& scene) const
{
	std::queue<Ray*> rays;
	Ray* currentRay = _head.get();
	rays.push(currentRay);

	size_t rayTreeCounter{ 0 };
	while (!rays.empty() && rayTreeCounter < maxTreeSize)
	{
		currentRay = rays.front();
		rays.pop();
		// The rayIntersection method adds intersection info to ray
		bool intersected = scene.rayIntersection(*currentRay);
		if (!intersected)
		{
			//std::cout << "A ray with no intersections detected\n";
			continue; // I think this is right...
		}

		auto& currentIntersection = currentRay->getIntersectionData().value();
		auto& currentIntersectObject = currentRay->getIntersectedObject().value();


		if (currentIntersectObject->getBRDF().getSurfaceType() == BRDF::REFLECTOR)
		{
			attachReflected(scene, currentIntersection, currentRay);
			// All importance is reflected
			currentRay->getLeft()->setColor(currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
		else if (currentIntersectObject->getBRDF().getSurfaceType() == BRDF::TRANSPARENT)
		{
			//std::cout << "uuuu\n";
			float incAngle = glm::angle(-currentRay->getNormalizedDirection(), currentIntersection._normal);

			// How much of the incoming importance/radiance is reflected, between 0 and 1.
			// The rest of the importance/radiance is transmitted.
			double reflectionCoeff;


			double n1, n2;
			if (currentRay->isInsideObject())
				n1 = _glassIndex, n2 = _airIndex;
			else
				n1 = _airIndex, n2 = _glassIndex;

			float brewsterAngle = asin(_airIndex/_glassIndex); // In radians // TODO Store this somewhere!

			if (currentRay->isInsideObject() && incAngle > brewsterAngle) // Total internal reflection
			{
				//std::cout << "Total internal reflection\n";
				reflectionCoeff = 1;
			}
			else // Transmission occurs
			{

				// Schlicks equation for radiance distribution
				double R0 = pow((n1 - n2) / (n1 + n2), 2);
				reflectionCoeff = R0 + (1 - R0) * pow(1.0 - cos(incAngle), 5);

				attachRefracted(scene, currentIntersection, currentRay);

				//if (currentRay->isInsideObject()) std::cout << "Transmission from inside object\n";

				// TODO BRDF should be used in these calculations
				currentRay->getRight()->setColor((1.0 - reflectionCoeff) * currentRay->getColor());
				//if (currentRay->getRight()->isInsideObject()) std::cout << "jasa\n";
				rays.push(currentRay->getRight());
				++rayTreeCounter;

			}

			// Always cast reflected ray
			attachReflected(scene, currentIntersection, currentRay);
			// TODO BRDF should be used in these calculations
			currentRay->getLeft()->setColor(reflectionCoeff * currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
	}
}

//double Scene::RayTree::findTotalShadow(Ray* input) const
//{
//	Ray* current = input;
//	while (current->getParent())
//	{
//		current = current->getParent();
//	}
//	return current->getShadow();
//}

Color Scene::RayTree::traverseRayTree(const Scene& scene, Ray* input) const
{
	Ray* current = input;
	Color result{};

	while (current != nullptr)
	{
		Ray* left = current->getLeft();
		Ray* right = current->getRight();

		// TODO Local lighting model contibutions

		if (left == nullptr && right == nullptr)
		{
			// TODO Ideally the ray should always intersect something
			if (current->getIntersectionData().has_value())
			{
				auto& intersectData = current->getIntersectionData().value();
				return current->getIntersectedObject().value()->getColor()
					* scene.shadowRayContribution(intersectData._intersectPoint,
												  intersectData._normal);
			}
			else
			{
				return Color{};
			}

		}
		else if (left && right == nullptr)
			current = current->getLeft();
		else if (left == nullptr && right)
			current = current->getRight();
		else if (left && right)
		{
			Color leftSubContrib = traverseRayTree(scene, current->getLeft()) * current->getLeft()->getColor();
			Color rightSubContrib = traverseRayTree(scene, current->getRight()) * current->getRight()->getColor();
			//return (leftSubContrib + rightSubContrib) / current->getColor();
			return (leftSubContrib + rightSubContrib);
		}
	}
	return result;
}

void Scene::RayTree::attachReflected(const Scene& scene, const IntersectionData& intData, Ray* currentRay) const
{
	Ray reflectedRay = scene.computeReflectedRay(
		intData._normal,
		*currentRay,
		intData._intersectPoint);

	// Reflected ray will always continue in the same medium
	reflectedRay.setInsideObject(currentRay->isInsideObject());

	//std::cout << std::string{ glm::to_string(reflectedRay.getStart()) + glm::to_string(reflectedRay.getEnd()) + '\n' };

	currentRay->setLeft(std::move(reflectedRay));
	currentRay->getLeft()->setParent(currentRay);
}

void Scene::RayTree::attachRefracted(const Scene& scene, const IntersectionData& intData, Ray* currentRay) const
{
	Ray refractedRay = scene.computeRefractedRay(
		intData._normal,
		*currentRay,
		intData._intersectPoint,
		currentRay->isInsideObject());

	// If refracting the medium changes
	refractedRay.setInsideObject(!currentRay->isInsideObject());

	currentRay->setRight(std::move(refractedRay));
	currentRay->getRight()->setParent(currentRay);
}
