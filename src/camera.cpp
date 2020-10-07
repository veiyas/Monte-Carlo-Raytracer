#include "camera.hpp"
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <chrono>
#include <list>

#include "ray.hpp"

Camera::Camera(bool eyePoint, int resolution)
	: _eyeToggle{ eyePoint }, WIDTH{ resolution }, HEIGHT{ resolution },
	  pixelSideLength{ 2.0f / resolution }, _gen{ std::random_device{}() }, _rng{ 0.f, 1.f }
{
}

void Camera::render(Scene& scene)
{
	std::cout << "Start rendering...\n";
	auto startTime = std::chrono::high_resolution_clock::now();

	//Get number of usable cores if possible, otherwise just use 1 thread
	size_t numCores = std::thread::hardware_concurrency();
	numCores = numCores == 0 ? 1 : numCores;

	std::cout << "Available threads: " << numCores << "\n";

	for (int row = 0; row < HEIGHT; row += numCores)
	{
		std::vector<std::thread> threads;
		
		for (size_t i = 0; (i < numCores) && (row + i < HEIGHT); i++)
		{
			threads.push_back(std::thread(&Camera::renderThreadFunction, this, row + i, std::ref(scene)));
		}
		
		for (auto& thread : threads)
		{
			thread.join();
		}
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = endTime - startTime;
	std::cout << "\nRendering finished, " << scene.getNCalculations() <<
		" calculations completed in " << duration.count() << " seconds\n\n";
}

void Camera::renderThreadFunction(int row, Scene& scene)
{
	// Give some indication of progress in a not so elegant way
	if (row % 50 == 0)
		std::cout << std::setw(2) << (100 * row) / WIDTH << "%\n";

	for (int col = 0; col < WIDTH; ++col)
	{
		for (int i = 0; i < _numOfRaysSentFromEachPixel; i++)
		{
			float yOffset = _rng(_gen);
			float zOffset = _rng(_gen);
			//float yOffset = 0.5f;
			//float zOffset = 0.5f;

			Vertex pixelPoint{
				0.0f,
				(col - (WIDTH / 2 + 1) + yOffset) * pixelSideLength,
				(row - (HEIGHT / 2 + 1) + zOffset) * pixelSideLength,
				1.0f
			};

			auto ray = std::make_shared<Ray>(_eyeToggle ? _eyePoint1 : _eyePoint2, pixelPoint, Color{ 1.0, 1.0, 1.0 });

			_pixels[row][col].addRay(ray);

			_pixels[row][col]._color += scene.raycastScene(*ray);
		}

	}
}

void Camera::createPNG()
{
	std::string filename = "output.png";

	double maxIntensity = 0.0f;
	for (size_t row = 0; row < HEIGHT; ++row)
	{
		for (size_t col = 0; col < WIDTH; ++col)
		{
			const Color& color = _pixels[row][col]._color;
			double maxOfPixel = glm::max(glm::max(color.r, color.g), color.b);
			if (maxOfPixel > maxIntensity)
				maxIntensity = maxOfPixel;
		}
	}

	std::cout << "Maximum intensity found: " << maxIntensity << '\n';

	std::cout << "Start writing to file...\n";
	
	std::vector<unsigned char> image;
	image.resize(WIDTH * HEIGHT * 4);

	for (size_t row = 0; row < HEIGHT; ++row)
	{
		for (size_t col = 0; col < WIDTH; col++)
		{
			unsigned char r = static_cast<unsigned char>(_pixels[row][col]._color.r * 255.99f / maxIntensity);
			unsigned char g = static_cast<unsigned char>(_pixels[row][col]._color.g * 255.99f / maxIntensity);
			unsigned char b = static_cast<unsigned char>(_pixels[row][col]._color.b * 255.99f / maxIntensity);			

			image[(HEIGHT - 1 - row) * 4 * WIDTH + (WIDTH - 1 - col) * 4 + 0] = r;
			image[(HEIGHT - 1 - row) * 4 * WIDTH + (WIDTH - 1 - col) * 4 + 1] = g;
			image[(HEIGHT - 1 - row) * 4 * WIDTH + (WIDTH - 1 - col) * 4 + 2] = b;
			image[(HEIGHT - 1 - row) * 4 * WIDTH + (WIDTH - 1 - col) * 4 + 3] = 255;
		}
	}

	unsigned error = lodepng::encode(filename, image, WIDTH, HEIGHT);
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	else
		std::cout << "Done!\n";
}
