#include "scene.hpp"

Scene::Scene()
{
	_tris.reserve(24);

	//Floor triangles
	for (size_t i = 0; i + 3 < floorVertices.size(); i += 3)
	{
		_tris.emplace_back(
			ceilingVertices[i], ceilingVertices[i + 1], ceilingVertices[i + 2],
			Direction{ 0.f, 0.f, 1.f }, Color{ 256.0, 256.0, 256.0 });
	}

	//Ceiling triangles
	for (size_t i = 0; i + 3 < floorVertices.size(); i += 3)
	{
		_tris.emplace_back(
			ceilingVertices[i], ceilingVertices[i + 1], ceilingVertices[i + 2],
			Direction{ 0.f, 0.f, -1.f }, Color{ 256.0, 256.0, 256.0 });
	}

	//Wall triangles
	size_t wallNormalCounter = 0;
	for (size_t i = 0; i + 3 < wallVertices.size(); i += 3)
	{
		_tris.emplace_back(
			wallVertices[i], wallVertices[i + 1], wallVertices[i + 2],
			wallNormals[wallNormalCounter], Color{ 0.0, 0.0, 0.0 });

		if (wallNormalCounter != 0 && (wallNormalCounter % 6 == 0))
			++wallNormalCounter;
	}
}

void Scene::intersections()
{
	//TODO
}
