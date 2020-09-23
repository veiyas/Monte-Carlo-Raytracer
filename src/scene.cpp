#include "scene.hpp"

Scene::Scene()
{
	_sceneTris.reserve(24);

	//Floor triangles
	for (size_t i = 0; i < floorVertices.size(); i += 3)
	{
		_sceneTris.emplace_back(
			floorVertices[i], floorVertices[i + 1], floorVertices[i + 2],
			Direction{ 0.f, 0.f, 1.f }, Color{ 0.84, 0.73, 0.45 });
	}

	//Ceiling triangles
	for (size_t i = 0; i < ceilingVertices.size(); i += 3)
	{
		_sceneTris.emplace_back(
			ceilingVertices[i], ceilingVertices[i + 1], ceilingVertices[i + 2],
			Direction{ 0.f, 0.f, -1.f }, Color{ 0.96, 0.37, 0.27 });
	}

	//Wall triangles
	size_t wallNormalCounter = 0;
	for (size_t i = 0; i < wallVertices.size(); i += 3)
	{
		//Since there are a lot more vertices than normals
		//Some care has to be taken when reading normals
		if (i % 6 == 0 && i != 0)
			wallNormalCounter++;

		_sceneTris.emplace_back(
			wallVertices[i], wallVertices[i + 1], wallVertices[i + 2],
			wallNormals[wallNormalCounter], Color{ 0.45, 0.58, 0.48 });

	}

	// Tetrahedrons
	//_tetrahedrons.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.80, 0.0, 0.80 }, Vertex{ 5.0f, 0.0f, 0.f, 1.0f });
	//_tetrahedrons.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.0f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 6.0f, -3.0f, -1.0f, 1.0f });
	// Intersecting the walls
	//_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 3.0f, Color{ 0.79, 0.0, 0.0 }, Vertex{ 13.0f, 0.0f, 0.0f, 1.0f });

	//Spheres
	_spheres.emplace_back(BRDF{ BRDF::REFLECTOR}, 2.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 0.f, -2.f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 9.f, 1.f, 0.f, 1.f });

	//Lights
	_pointLights.emplace_back(Vertex(0, 3, 4, 1), Color(1, 1, 1));
}

Color Scene::raycastScene(Ray& initialRay)
{
	RayTree tree{ initialRay };
	tree.raytrace(*this);
	return tree.getPixelColor();
}

std::pair<Triangle, unsigned> Scene::rayIntersection(Ray& ray)
{
	Vertex closestIntersectPoint{};
	Direction closestIntersectNormal{};
	Color closestIntersectColor{};
	unsigned closestIntersectSurfaceType = BRDF::DIFFUSE;
	float minT = 1e+10;

	//TODO put these in single loop
	for (auto& triangle : _sceneTris)
	{
		float t = triangle.rayIntersection(ray);
		++_nCalculations;
		if (t != -1 && t < minT)
		{
			closestIntersectPoint =
				ray.getStart() + glm::normalize((ray.getEnd() - ray.getStart())) * t;
			closestIntersectNormal = triangle.getNormal();
			closestIntersectColor = triangle.getColor();
			minT = t;
		}
	}

	for (auto& tetrahedron : _tetrahedrons)
	{
		auto intersect = tetrahedron.rayIntersection(ray);
		++_nCalculations;
		if (intersect.first != -1 && intersect.first < minT)
		{
			closestIntersectPoint =
				ray.getStart() + glm::normalize((ray.getEnd() - ray.getStart())) * intersect.first;
			closestIntersectNormal = intersect.second.getNormal();
			closestIntersectColor = intersect.second.getColor();
			closestIntersectSurfaceType = tetrahedron.getBRDF().getSurfaceType();
			minT = intersect.first;
		}
	}

	for (auto& sphere : _spheres)
	{
		auto intersect = sphere.rayIntersection(ray);
		++_nCalculations;
		if (intersect.first != -1 && intersect.first < minT)
		{
			closestIntersectPoint =
				ray.getStart() + glm::normalize((ray.getEnd() - ray.getStart())) * intersect.first;
			closestIntersectNormal = intersect.second.getNormal();
			closestIntersectColor = intersect.second.getColor();
			closestIntersectSurfaceType = sphere.getBRDF().getSurfaceType();
			minT = intersect.first;
		}
	}
	float shadow = shadowRayContribution(closestIntersectPoint, closestIntersectNormal);
	closestIntersectColor *= (shadow + _ambientContribution);

	Triangle intersectedTri{ closestIntersectPoint, closestIntersectNormal, closestIntersectColor };
	ray.setColor(closestIntersectColor);
	ray.setEndTriangle(intersectedTri);
	return std::make_pair(intersectedTri, closestIntersectSurfaceType);
;
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
		lightContribution += normalDotContribution * visible;
	}
	return lightContribution;
}

bool Scene::objectIsVisible(const std::pair<float, Triangle>& input, const Direction& normal) const
{
	if (input.first != -1 // Intersection must exist
		&& input.first < 1 // Intersections with t > 1 are behind the light
		&& normal != input.second.getNormal())
		return false;
	else
		return true;
}

Ray Scene::computeReflectedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint) const
{
	Direction incomingRayDirection =
		glm::normalize(incomingRay.getEnd() - incomingRay.getStart());
	//Angle between normal and incoming ray
	float angle = glm::angle(normal, incomingRayDirection);

	Direction reflectedDirection =
		incomingRayDirection - 2.f * (glm::dot(incomingRayDirection, normal)) * normal;

	return Ray{ Vertex{ intersectionPoint },
	            Vertex{ glm::vec3(intersectionPoint) + reflectedDirection, 1.f } };
}

Ray Scene::computeRefractedRay(const Direction& normal, const Ray& incomingRay, const Vertex& intersectionPoint, bool insideObject) const
{
	Direction incomingDir = glm::normalize(incomingRay.getEnd() - incomingRay.getStart());
	float n1, n2;
	if (insideObject)
	{
		n1 = 1.2f;
		n2 = 1.f;
	}
	else
	{
		n1 = 1.f;
		n2 = 1.2f;
	}
	float n1n2 = n1 / n2;
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
	_treeSize = 1;
	_reflectionContrib = 1.0 - _transmissionContrib;
}

void Scene::RayTree::raytrace(Scene& scene)
{
	auto currentIntersection = scene.rayIntersection(*_head.get());

	if (currentIntersection.second == BRDF::DIFFUSE) //Hit wall immidiately
	{
		_finalColor = _head->getColor();
	}
	else
	{
		constructRayTree(scene);
		_finalColor = traverseRayTree(scene, _head.get());
	}
}

void Scene::RayTree::constructRayTree(Scene& scene) const
{
	Ray* currentRay{};
	std::pair<Triangle, unsigned> currentIntersection{};

	
	size_t rayTreeCounter{ 0 };

	std::queue<Ray*> rays;
	rays.push(_head.get());
	while (!rays.empty() && rayTreeCounter < maxTreeSize)
	{
		currentIntersection = scene.rayIntersection(*rays.front());
		currentRay = rays.front();
		rays.pop();

		if (currentIntersection.second == BRDF::REFLECTOR)
		{
			Ray reflectedRay = scene.computeReflectedRay(
				currentIntersection.first.getNormal(),
				*currentRay,
				currentIntersection.first.getPoint());

			currentRay->setLeft(std::move(reflectedRay));
			currentRay->getLeft()->setParent(currentRay);

			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
		else if (currentIntersection.second == BRDF::TRANSPARENT)
		{
			Ray reflectedRay = scene.computeReflectedRay(
				currentIntersection.first.getNormal(),
				*currentRay,
				currentIntersection.first.getPoint());

			currentRay->setLeft(std::move(reflectedRay));
			currentRay->getLeft()->setParent(currentRay);

			rays.push(currentRay->getLeft());
			++rayTreeCounter;

			bool willRefract = false;
			if (currentRay->isInsideObject())
			{
				//Check brewster angle
				Direction rayDir =
					glm::vec3(glm::normalize(currentRay->getEnd() - currentRay->getStart()));
				if (glm::angle(currentIntersection.first.getNormal(), rayDir) < 42.f)
				{
					willRefract = true;
				}
			}
			else if(!currentRay->isInsideObject() || willRefract)
			{
				Ray refractedRay = scene.computeRefractedRay(
					currentIntersection.first.getNormal(),
					*currentRay,
					currentIntersection.first.getPoint(), currentRay->isInsideObject());
				refractedRay.setInsideObject(true);

				currentRay->setRight(std::move(refractedRay));
				currentRay->getRight()->setParent(currentRay);
				rays.push(currentRay->getRight());

				++rayTreeCounter;
			}
		}
	}
}

Color Scene::RayTree::traverseRayTree(const Scene& scene, Ray* input) const
{
	Ray* current = input;
	Color result{};

	while (current != nullptr)
	{
		Ray* left = current->getLeft();
		Ray* right = current->getRight();

		if (left == nullptr && right == nullptr)
			return  current->getEndTriangle()->getColor();
		else if (left && right == nullptr)
			current = current->getLeft();
		else if (left == nullptr && right)
			current = current->getRight();
		else if (left && right)
		{
			Color leftSubTreeColor = traverseRayTree(scene, current->getLeft()) * _reflectionContrib;
			Color rightSubTreeColor = traverseRayTree(scene, current->getRight()) * _transmissionContrib;
			return leftSubTreeColor + rightSubTreeColor;
		}
	}
	return result;
}
