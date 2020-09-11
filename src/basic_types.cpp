#include "basic_types.hpp"
#include <glm/glm.hpp>

std::vector<Vertex> Ray::_imagePlaneVertices;

Ray::Ray(Vertex start, Vertex end)
{
	_imagePlaneVertices.push_back(std::move(start));
	_imagePlaneVertices.push_back(std::move(end));

	_start = std::make_unique<Vertex>(start);
	_end = std::make_unique<Vertex>(end);
}

void Ray::initVertexList()
{
	_imagePlaneVertices.reserve(72);
}

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color)
	: _v1{ v1 }, _v2{ v2 }, _v3{ v3 }, _normal{ normal }, _color{ color }
{

}

bool Triangle::rayIntersection(Ray& arg) const
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

	return (u >= 0) && (v >= 0) && (u + v <= 1) && (t > 0);
}
