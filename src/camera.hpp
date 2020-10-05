#pragma once

#include <vector>
#include <random>

#include "lodepng.h"

#include "basic_types.hpp"
#include "scene.hpp"

class Camera
{
public:
	Camera(bool eyePoint = false);

	void render(Scene& scene);
	void createPNG();

private:
	const Vertex _eyePoint1{ -2.0f, 0.0f, 0.0f, 1.0f };
	const Vertex _eyePoint2{ -1.0f, 0.0f, 0.0f, 1.0f };

	static constexpr float pixelSideLength = 0.0025f;

	// Decides which of the two eye points is used
	bool _eyeToggle = false;

	// The number of pixels in each direction
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 800;
	
	using PixelGrid = std::vector<std::vector<Pixel>>;
	PixelGrid _pixels{ HEIGHT, std::vector<Pixel>(WIDTH) };

	// Random generator stuff
	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;

	void renderThreadFunction(int row, Scene& scene);
};
