#include "scenegeometry.hpp"

SceneGeometry::SceneGeometry()
{
	std::cout << "Constructing scene...   ";
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
	_ceilingLights.emplace_back(BRDF{ BRDF::LIGHT }, 7.f, 0.f);

	//// Algots scene
	//_tetrahedrons.emplace_back(BRDF{ BRDF::REFLECTOR }, 0.8f, Color{ 1.0, 0.0, 0.0 }, Vertex{ 3.0f, 2.0f, -1.0f, 1.0f });
	//_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.5f, Color{ 0.1, 0.1, 1.0 }, Vertex{ 8.f, 0.f, -2.5f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 5.f, 0.f, 0.f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::REFLECTOR }, 1.5f, Color{ 0.02, 0.02, 0.02 }, Vertex{ 9.f, 0.0f, -3.5f, 1.f });
	//_spheres.emplace_back(BRDF{ BRDF::TRANSPARENT }, 1.5f, Color{ 0.1, 0.1, 0.1 }, Vertex{ 6.f, 3.5f, -3.f, 1.f });
	_spheres.emplace_back(BRDF{ BRDF::DIFFUSE }, 1.5f, Color{ 1.0, 1.0, 1.0 }, Vertex{ 6.f, -3.5f, -3.f, 1.f });
	std::cout << "done!\n";
}
