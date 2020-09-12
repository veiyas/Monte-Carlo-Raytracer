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
		_sceneTris.emplace_back(
			wallVertices[i], wallVertices[i + 1], wallVertices[i + 2],
			wallNormals[wallNormalCounter], Color{ 0.45, 0.58, 0.48 });

		//Since there are a lot more vertices than normals
		//Some care has to be taken when reading normals
		if (i % 6 == 0 && i != 0)
			wallNormalCounter++;
	}
}

void Scene::intersections(Ray& ray, std::list<Triangle*>& intersectingTriangles)
{
	for (auto& triangle : _sceneTris)
	{
		if (triangle.rayIntersection(ray))
		{
			intersectingTriangles.push_back(&triangle);
		}
	}	
}
