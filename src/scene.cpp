#include "scene.hpp"

Scene::Scene()
{
	_sceneTris.reserve(24);

	//Floor triangles
	for (size_t i = 0; i < floorVertices.size(); i += 3)
	{
		_sceneTris.emplace_back(
			floorVertices[i], floorVertices[i + 1], floorVertices[i + 2],
			Direction{ 0.f, 0.f, 1.f }, Color{ 0.2, 0.3, 0.97 });
	}

	//Ceiling triangles
	for (size_t i = 0; i < ceilingVertices.size(); i += 3)
	{
		_sceneTris.emplace_back(
			ceilingVertices[i], ceilingVertices[i + 1], ceilingVertices[i + 2],
			Direction{ 0.f, 0.f, -1.f }, Color{ 0.96, 0.37, 0.27 });
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
			wallVertices[i], wallVertices[i + 1], wallVertices[i + 2],
			wallNormals[wallNormalCounter], wallColors[(i/3)/2]);

	}

	// Tetrahedrons
	//_tetrahedrons.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.80, 0.0, 0.80 }, Vertex{ 5.0f, 0.0f, 0.f, 1.0f });
	//_tetrahedrons.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.0f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 6.0f, -3.0f, -1.0f, 1.0f });
	// Intersecting the walls
	//_tetrahedrons.emplace_back(BRDF{ BRDF::DIFFUSE }, 3.0f, Color{ 0.79, 0.0, 0.0 }, Vertex{ 13.0f, 0.0f, 0.0f, 1.0f });

	//Spheres
	//_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 2.f, 0.f, 0.f, 1.f });
	_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 2.f, -2.f, 1.f });
	_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 2.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 7.f, -2.f, -1.f, 1.f });

	//Lights
	_pointLights.emplace_back(Vertex(1, -1, 1, 1), Color(1, 1, 1));
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
	//double shadow = 1.f;
	//if (closestIntersectSurfaceType != BRDF::TRANSPARENT)
	//{
	//	shadow = shadowRayContribution(closestIntersectPoint, closestIntersectNormal);
	//	closestIntersectColor *= (shadow + _ambientContribution);
	//}
	//else
	//	closestIntersectColor *= _ambientContribution;

	// TODO Not sure returning a tri makes sense when it could just as well be a sphere
	Triangle intersectedTri{ closestIntersectPoint, closestIntersectNormal, closestIntersectColor };
	//ray.setColor(closestIntersectColor); // The color member is used for importance and should not be set like this
	//ray.setShadow(shadow); // TODO What is this used for?
	ray.setEndTriangle(intersectedTri);
	ray.setIntersectPoint(closestIntersectPoint);
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
	_head->setShadow(1.0);
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

	std::pair<Triangle, unsigned> currentIntersection{};	
	size_t rayTreeCounter{ 0 };
	while (!rays.empty() && rayTreeCounter < maxTreeSize)
	{
		currentRay = rays.front();
		currentIntersection = scene.rayIntersection(*currentRay);
			//if (currentRay->isInsideObject()) std::cout << (currentIntersection.second == BRDF::DIFFUSE) << "\n";
			//if (currentRay->isInsideObject()) std::cout << currentIntersection.second << "\n";
		rays.pop();

		if (currentIntersection.second == BRDF::REFLECTOR)
		{
			attachReflected(scene, currentIntersection.first, currentRay);
			currentRay->getLeft()->setColor(currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
		else if (currentIntersection.second == BRDF::TRANSPARENT)
		{
			//std::cout << "uuuu\n";
			float incAngle = glm::angle(-currentRay->getNormalizedDirection(), currentIntersection.first.getNormal());

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

				attachRefracted(scene, currentIntersection.first, currentRay);

				//if (currentRay->isInsideObject()) std::cout << "Transmission from inside object" << '\n';

				// TODO BRDF should be used in these calculations
				currentRay->getRight()->setColor((1.0 - reflectionCoeff) * currentRay->getColor());
				//if (currentRay->getRight()->isInsideObject()) std::cout << "jasa\n";
				rays.push(currentRay->getRight());
				++rayTreeCounter;

			}

			// Always cast reflected ray
			attachReflected(scene, currentIntersection.first, currentRay);
			// TODO BRDF should be used in these calculations
			currentRay->getLeft()->setColor(reflectionCoeff * currentRay->getColor());
			rays.push(currentRay->getLeft());
			++rayTreeCounter;
		}
	}
}

double Scene::RayTree::findTotalShadow(Ray* input) const
{
	Ray* current = input;
	while (current->getParent())
	{
		current = current->getParent();
	}
	return current->getShadow();
}

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
			// TODO This probably shouldnt be needed at all,
			// try to find the source of the error
			if (!current->getEndTriangle())
				return Color{};

			// TODO Remake with some better handling of tri/sphere
			return current->getEndTriangle()->getColor()
				* scene.shadowRayContribution(current->getIntersectionPoint(),
					                          current->getEndTriangle()->getNormal());
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

void Scene::RayTree::attachReflected(const Scene& scene, Triangle& hitTri, Ray* currentRay) const
{
	Ray reflectedRay = scene.computeReflectedRay(
		hitTri.getNormal(),
		*currentRay,
		hitTri.getPoint());

	// Reflected ray will always continue in the same medium
	reflectedRay.setInsideObject(currentRay->isInsideObject());

	currentRay->setLeft(std::move(reflectedRay));
	currentRay->getLeft()->setParent(currentRay);
}

void Scene::RayTree::attachRefracted(const Scene& scene, Triangle& hitTri, Ray* currentRay) const
{
	Ray refractedRay = scene.computeRefractedRay(
		hitTri.getNormal(),
		*currentRay,
		hitTri.getPoint(), currentRay->isInsideObject());

	// If refracting the medium changes
	refractedRay.setInsideObject(!currentRay->isInsideObject());

	currentRay->setRight(std::move(refractedRay));
	currentRay->getRight()->setParent(currentRay);
}
