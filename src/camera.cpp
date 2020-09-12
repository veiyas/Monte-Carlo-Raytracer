#include "camera.hpp"
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <chrono>
#include <list>

Camera::Camera(bool eyePoint) : _eyeToggle{ eyePoint }
{
	std::random_device rd;
	gen = std::mt19937(rd());
	rng = std::uniform_real_distribution<float>(0.0f, 1.0f);
}

void Camera::render(Scene& scene)
{
	std::cout << "Start rendering...\n";
	auto startTime = std::chrono::high_resolution_clock::now();

	//Get number of usable cores if possible, otherwise just use 1 thread
	size_t numCores = std::thread::hardware_concurrency();
	numCores = numCores == 0 ? 1 : numCores;

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
	std::cout << "Rendering finished in " << duration.count() << " seconds\n\n";
}

void Camera::renderThreadFunction(int row, Scene& scene)
{
	// Give some indication of progress in a not so elegant way
	if (row % 50 == 0)
		std::cout << (100 * row) / WIDTH << " percent done\n";

	for (int col = 0; col < WIDTH; ++col)
	{
		float yOffset = rng(gen);
		float zOffset = rng(gen);

		Vertex pixelPoint{
			0.0f,
			(col - (WIDTH / 2 + 1) + yOffset) * pixelSideLength,
			(row - (HEIGHT / 2 + 1) + zOffset) * pixelSideLength,
			1.0f
		};

		auto ray = std::make_shared<Ray>(_eyeToggle ? _eyePoint1 : _eyePoint2, pixelPoint);

		_pixels.at(row).at(col).addRay(ray);

		// Assume for now that only one intersection takes place and give the color
		// of the intersected triangle to the pixel
		std::list<Triangle*> intersections;
		scene.intersections(*ray, intersections);
		// It probably shouldnt even be possible for this to be empty in a closed scene...
		if (!intersections.empty())
		{
			if (intersections.size() != 1)
				std::cout << "Mulitple intersections!\n";
			_pixels[row][col]._color = intersections.front()->getColor();
		}
		else
		{
			_pixels[row][col]._color = Color{ 0.0 };
			std::cout << "No intersections!\n";
		}
	}
}

void Camera::createImage()
{
	// This code assumes the whole scene is well-lit.
	// See p.13 of lecture 6

	double maxIntensity = 0.0f;
	for (size_t row = 0; row < HEIGHT; ++row)
		for (size_t col = 0; col < WIDTH; ++col)
		{
			const Color& color = _pixels[row][col]._color;
			double maxOfPixel = glm::max(glm::max(color.r, color.g), color.b);
			if (maxOfPixel > maxIntensity)
				maxIntensity = maxOfPixel;
		}
	std::cout << "Maximum intensity found: " << maxIntensity << '\n';

	std::cout << "Start writing to file\n";

	std::ofstream redFile("testingRed.csv", std::ofstream::trunc);
	std::ofstream greenFile("testingGreen.csv", std::ofstream::trunc);
	std::ofstream blueFile("testingBlue.csv", std::ofstream::trunc);

	for (size_t row = 0; row < HEIGHT; ++row)
	{
		for (size_t col = 0; col < WIDTH; ++col)
		{
			redFile   << (int)(_pixels[row][col]._color.r * 255.99f / maxIntensity) << ((col != HEIGHT-1) ? "," : "");
			greenFile << (int)(_pixels[row][col]._color.g * 255.99f / maxIntensity) << ((col != HEIGHT-1) ? "," : "");
			blueFile  << (int)(_pixels[row][col]._color.b * 255.99f / maxIntensity) << ((col != HEIGHT-1) ? "," : "");
		}
		redFile   << '\n';
		greenFile << '\n';
		blueFile  << '\n';
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

			image[row * 4 * WIDTH + col * 4 + 0] = r;
			image[row * 4 * WIDTH + col * 4 + 1] = g;
			image[row * 4 * WIDTH + col * 4 + 2] = b;
			image[row * 4 * WIDTH + col * 4 + 3] = 255;
		}
	}

	unsigned error = lodepng::encode(filename, image, WIDTH, HEIGHT);
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	else
		std::cout << "Done!\n";
}
