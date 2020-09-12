#include "basic_types.hpp"
#include <glm/glm.hpp>

std::vector<Vertex> Ray::_imagePlaneVertices;

Ray::Ray(Vertex start, Vertex end)
{
	_start = std::make_unique<Vertex>(start);
	_end = std::make_unique<Vertex>(end);

	//TODO this needs to be resolved, concurrency errors probably
	//const std::lock_guard<std::mutex> lock(arrayLock);
	//_imagePlaneVertices.push_back(start);
	//_imagePlaneVertices.push_back(end);
}

void Ray::initVertexList()
{
	_imagePlaneVertices.reserve(72);
}

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color)
	: _v1{ v1 }, _v2{ v2 }, _v3{ v3 }, _normal{ normal }, _color{ color }
{

}

float Triangle::rayIntersection(Ray& arg) const
{
	// Moller Trumbore algorithm
	glm::vec3 T = arg.getStart() - _v1;
	glm::vec3 E1 = _v2 - _v1;
	glm::vec3 E2 = _v3 - _v1;
	glm::vec3 D = arg.getEnd() - arg.getStart();
	glm::vec3 P = glm::cross(glm::vec3(D), glm::vec3(E2));
	glm::vec3 Q = glm::cross(glm::vec3(T), glm::vec3(E1));

	float factor = 1 / glm::dot(P, E1);

	float t = factor * glm::dot(Q, E2);
	float u = factor * glm::dot(P, T);
	float v = factor * glm::dot(Q, D);

	bool pointIsOnTriangle = (u >= 0) && (v >= 0) && (u + v <= 1);

	// Return t for the intersection or -1 if no intersection is found
	return (pointIsOnTriangle && t > 0) ? t : -1;
}


Tetrahedron::Tetrahedron(float radius, Color color, Vertex position)
{
	_triangles.reserve(4);

	const Vertex v[4] = {
		radius * Vertex{  1.0f,  1.0f,  1.0f, 1.0f } + position,
		radius * Vertex{ -1.0f, -1.0f,  1.0f, 1.0f } + position,
		radius * Vertex{ -1.0f,  1.0f, -1.0f, 1.0f } + position,
		radius * Vertex{  1.0f, -1.0f, -1.0f, 1.0f } + position
	};

	// TODO Fix direction -- maybe do automatically in triangle
	_triangles.emplace_back(v[2], v[1], v[0], Direction{ }, color);
	_triangles.emplace_back(v[0], v[3], v[2], Direction{ }, color);
	_triangles.emplace_back(v[1], v[3], v[0], Direction{ }, color);
	_triangles.emplace_back(v[2], v[3], v[1], Direction{ }, color);
}

Color Tetrahedron::getColor() const
{
	return _triangles[0].getColor();
}

std::pair<float, Color> Tetrahedron::rayIntersection(Ray& ray) const
{
	const Triangle* closestIntersectingTriangle = nullptr;
	float minT = 1e+10;
	for (auto& triangle : _triangles)
	{
		float t = triangle.rayIntersection(ray);
		if (t != -1 && t < minT)
		{
			closestIntersectingTriangle = &triangle;
			minT = t;
		}
	}

	if (closestIntersectingTriangle != nullptr)
		return std::make_pair(minT, closestIntersectingTriangle->getColor());
	else
		return std::make_pair(-1, Color{ });
}
