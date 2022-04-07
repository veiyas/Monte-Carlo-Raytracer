#include "triangle.hpp"

#include <glm/glm.hpp>

#include "ray.hpp"

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color)
	: _v1{ v1 }, _v2{ v2 }, _v3{ v3 }, _normal{ normal }, _color{ color }
{

}

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3, Color color)
	: _v1{ v1 }, _v2{ v2 }, _v3{ v3 }, _color{ color }
{
	glm::vec3 v2v1 = glm::normalize(glm::vec3(v2 - v1));
	glm::vec3 v3v2 = glm::normalize(glm::vec3(v3 - v2));
	_normal = glm::cross(v2v1, v3v2);
}

Vertex Triangle::getCenter() const
{
	return Vertex{
		(_v1.x + _v2.x + _v3.x) / 3,
		(_v1.y + _v2.y + _v3.y) / 3,
		(_v1.z + _v2.z + _v3.z) / 3,
		1.f };
}

float Triangle::rayIntersection(const Ray& arg) const
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
	return (pointIsOnTriangle && t > 0) ? t * glm::length(D) : -1;
}