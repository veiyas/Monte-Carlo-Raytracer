#include "camera.hpp"
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

Camera::Camera(bool eyePoint) : _eyeToggle{ eyePoint }
{
	std::random_device rd;
	gen = std::mt19937(rd());
	rng = std::uniform_real_distribution<float>(0.0f, 1.0f);
}

void Camera::render()
{
	for (size_t i = 0; i < WIDTH; ++i)
		for (size_t j = 0; j < HEIGHT; ++j)
		{
			float yOffset = rng(gen);
			float zOffset = rng(gen);

			Vertex pixelPoint{
				0.0f,
				(i - (WIDTH / 2 + 1) + yOffset) * pixelSideLength,
				(j - (HEIGHT / 2 + 1) + yOffset) * pixelSideLength,
				1.0f
			};

			auto ray = std::make_shared<Ray>(_eyeToggle ? _eyePoint1 : _eyePoint2, pixelPoint);

			_pixels[i][j].addRay(ray);
			_pixels[i][j]._color = Color{ rng(gen), rng(gen), rng(gen) }; // TODO pixel color
		}
}

void Camera::createImage()
{
	// This code assumes the whole scene is well-lit.
	// See p.13 of lecture 6

	double maxIntensity = 0.0f;
	for (size_t i = 0; i < WIDTH; ++i)
		for (size_t j = 0; j < HEIGHT; ++j)
		{
			const Color& col = _pixels[i][j]._color;
			double maxOfPixel = glm::max(glm::max(col.r, col.g), col.b);
			if (maxOfPixel > maxIntensity)
				maxIntensity = maxOfPixel;
		}

	std::cout << "Start writing to file\n";

	std::ofstream redFile("testingRed.csv", std::ofstream::trunc);
	std::ofstream greenFile("testingGreen.csv", std::ofstream::trunc);
	std::ofstream blueFile("testingBlue.csv", std::ofstream::trunc);

	for (size_t i = 0; i < WIDTH; ++i)
	{
		for (size_t j = 0; j < HEIGHT; ++j)
		{
			redFile   << (int)(_pixels[i][j]._color.r * 255.99f / maxIntensity) << ((j != HEIGHT-1) ? "," : "");
			greenFile << (int)(_pixels[i][j]._color.g * 255.99f / maxIntensity) << ((j != HEIGHT-1) ? "," : "");
			blueFile  << (int)(_pixels[i][j]._color.b * 255.99f / maxIntensity) << ((j != HEIGHT-1) ? "," : "");
		}
		redFile   << '\n';
		greenFile << '\n';
		blueFile  << '\n';
	}

}