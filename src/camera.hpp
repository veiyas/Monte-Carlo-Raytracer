#pragma once

#include <vector>
#include <random>

#include "basic_types.hpp"
#include "scene.hpp"
#include "config.hpp"

class Camera
{
public:
	Camera();

	void render(Scene& scene);
	void sqrtAllPixels();
	void createPNG(const std::string& file);

private:
	const Vertex _eyePoint1{ -2.0f, 0.0f, 0.0f, 1.0f };
	const Vertex _eyePoint2{ -1.0f, 0.0f, 0.0f, 1.0f };

	// The number of pixels in each direction
	const int WIDTH;
	const int HEIGHT;
	const float pixelSideLength;
	
	using PixelGrid = std::vector<std::vector<Pixel>>;
	PixelGrid _pixels{ HEIGHT, std::vector<Pixel>(WIDTH) };

	// Random generator stuff
	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;

	void renderThreadFunction(int row, Scene& scene);
};
