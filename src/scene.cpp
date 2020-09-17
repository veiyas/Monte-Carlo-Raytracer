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

	//_tetrahedrons.emplace_back(BRDF{ BRDF::REFLECTOR }, 2.0f, Color{ 0.15, 0.98, 0.38 }, Vertex{ 9.0f, -3.0f, 0.0f, 1.0f });
	//_tetrahedrons.emplace_back(BRDF{ BRDF::REFLECTOR }, 2.0f, Color{ 0.80, 0.0, 0.80 }, Vertex{ 9.0f, -1.0f, 2.0f, 1.0f });
	//_tetrahedrons.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.0f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 6.0f, -3.0f, -1.0f, 1.0f });
	// Intersecting the walls
	//_tetrahedrons.emplace_back(3.0f, Color{ 0.79, 0.0, 0.0 }, Vertex{ 13.0f, 0.0f, 0.0f, 1.0f });


	//Spheres
	//_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 9.f, 0.f, 3.f, 1.f }, 1.f);
	_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 6.f, -1.f, 0.f, 1.f }, 1.f);
	//_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 9.f, 3.f, -3.f, 1.f }, 1.f);
	//_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 9.f, -3.f, 3.f, 1.f }, 1.f);
	//_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 9.f, 0.f, 3.f, 1.f }, 1.f);
	//_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 9.f, 3.f, 3.f, 1.f }, 1.f);

	//Lights
	_pointLights.emplace_back(Vertex(3, 5, 0, 1), Color(1, 1, 1));
}

Color Scene::intersection(Ray& ray) const
{
	Vertex closestIntersectPoint{};
	Direction closestIntersectNormal{};
	Color closestIntersectColor{};
	float minT = 1e+10;

	//TODO put these in single loop
	for (auto& triangle : _sceneTris)
	{
		float t = triangle.rayIntersection(ray);
		if (t != -1 && t < minT)
		{
			closestIntersectPoint =
				ray.getStart() + glm::normalize((ray.getEnd() - ray.getStart())).operator*=(t);
			closestIntersectNormal = triangle.getNormal();
			closestIntersectColor = triangle.getColor();
			minT = t;
		}
	}

	for (auto& tetrahedron : _tetrahedrons)
	{
		auto intersect = tetrahedron.rayIntersection(ray);
		if (intersect.first != -1 && intersect.first < minT)
		{
			closestIntersectPoint =
				ray.getStart() + glm::normalize((ray.getEnd() - ray.getStart())).operator*=(intersect.first);
			closestIntersectNormal = intersect.second.getNormal();
			closestIntersectColor = intersect.second.getColor();
			minT = intersect.first;
		}
	}

	for (auto& sphere : _spheres)
	{
		auto intersect = sphere.rayIntersection(ray);
		if (intersect.first != -1 && intersect.first < minT)
		{
			closestIntersectPoint =
				ray.getStart() + glm::normalize((ray.getEnd() - ray.getStart())).operator*=(intersect.first);
			closestIntersectNormal = intersect.second.getNormal();
			closestIntersectColor = intersect.second.getColor();
			minT = intersect.first;
		}
	}
	float shadow = shadowRayContribution(closestIntersectPoint, closestIntersectNormal);
	//if(shadow < 0.1f)
		//std::cout << "Shadow: " << shadow << "\n";

	closestIntersectColor *= (shadow + ambientContribution);
	return closestIntersectColor;
}

float Scene::shadowRayContribution(const Vertex& point, const Direction& normal) const
{
	float lightContribution = 0.f;

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
