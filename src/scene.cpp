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
	//_tetrahedrons.emplace_back(2.0f, Color{ 0.15, 0.98, 0.38 }, Vertex{ 9.0f, 4.0f, 0.0f, 1.0f });
	//_tetrahedrons.emplace_back(2.0f, Color{ 0.80, 0.0, 0.80 }, Vertex{ 9.0f, -1.0f, 0.0f, 1.0f });
	//_tetrahedrons.emplace_back(1.0f, Color{ 0.0, 0.50, 0.94 }, Vertex{ 6.0f, -3.0f, -1.0f, 1.0f });
	// Intersecting the walls
	//_tetrahedrons.emplace_back(3.0f, Color{ 0.79, 0.0, 0.0 }, Vertex{ 13.0f, 0.0f, 0.0f, 1.0f });

	//Spheres
	_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 0.f, -3.f, 1.f }, 1.f);
	_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, -3.f, -3.f, 1.f }, 1.f);
	_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 3.f, -3.f, 1.f }, 1.f);
	_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, -3.f, 3.f, 1.f }, 1.f);
	_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 0.f, 3.f, 1.f }, 1.f);
	_spheres.emplace_back(1.f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 5.f, 3.f, 3.f, 1.f }, 1.f);

	//Lights
	_pointLights.emplace_back(Vertex(5, 0, 0, 1), Color(1, 1, 1));
}

Color Scene::intersection(Ray& ray)
{
	Color colorOfClosestIntersect{ };
	float minT = 1e+10;

	for (auto& triangle : _sceneTris)
	{
		float t = triangle.rayIntersection(ray);
		if (t != -1 && t < minT)
		{
			colorOfClosestIntersect =
				triangle.getColor().operator*=(shadowRayContribution(triangle));
			minT = t;
		}
	}

	for (auto& tetrahedron : _tetrahedrons)
	{
		auto intersect = tetrahedron.rayIntersection(ray);
		if (intersect.first != -1 && intersect.first < minT)
		{
			colorOfClosestIntersect =
				intersect.second.getColor()
				.operator*=(shadowRayContribution(intersect.second));
			minT = intersect.first;
		}
	}

	for (auto& sphere : _spheres)
	{
		auto intersect = sphere.rayIntersection(ray);
		
		if (intersect.first != -1 && intersect.first < minT)
		{
			float shadowRatio = shadowRayContribution(intersect.second);
			colorOfClosestIntersect = sphere.getColor().operator*=(shadowRatio);
			minT = intersect.first;
		}
	}

	// Since a closed scene is used, there should always be at least one intersection
	return colorOfClosestIntersect;
}

float Scene::shadowRayContribution(const Triangle& tri) const
{
	float lightContribution = 0.f;
	for (const auto& light : _pointLights)
	{
		bool visible = true;
		Vertex shadowRay = light.getPosition() - tri.getCenter();
		Direction shadowRayVec = glm::normalize(glm::vec3(shadowRay));

		//TODO perform visibility test

		if (visible)
			lightContribution += glm::max(0.2f, glm::dot(shadowRayVec, tri.getNormal()));
		else
			lightContribution += 0.f;
	}
	return lightContribution;
}