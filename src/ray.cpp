#include "ray.hpp"

#include <glm/glm.hpp>

std::vector<Vertex> Ray::_imagePlaneVertices;

Ray::Ray(Vertex start, Vertex end)
{
	_start = std::make_unique<Vertex>(start);
	_end = std::make_unique<Vertex>(end);
	_shadow = 0.0;
	//TODO this needs to be resolved, concurrency errors probably
	//const std::lock_guard<std::mutex> lock(arrayLock);
	//_imagePlaneVertices.push_back(start);
	//_imagePlaneVertices.push_back(end);
}

Ray::Ray(Vertex start, Vertex end, Color color)
	: Ray{ start, end }
{
	_rayColor = color;
}

Ray::Ray(Ray& ray)
{
	// TODO Using std::move when making a copy probably isnt right
	_left = std::move(ray._left);
	_right = std::move(ray._right);
	_start = std::move(ray._start);
	_end = std::move(ray._end);
	_endTriangle = std::move(ray._endTriangle);

	_rayColor = ray._rayColor;
	_isInsideObject = ray._isInsideObject;
}

void Ray::initVertexList()
{
	_imagePlaneVertices.reserve(72);
}

Direction Ray::getNormalizedDirection() const
{
	// TODO Consider storing the direction as a member
	return Direction{ glm::normalize(getEnd() - getStart()) };
}