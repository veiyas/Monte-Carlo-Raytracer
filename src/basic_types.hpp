#pragma once

#include <vector>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

typedef glm::vec4 Vertex;
typedef glm::vec3 Direction;
typedef glm::vec<3, double> Color;

class Ray
{
public:
	Ray(Vertex start, Vertex end);

	static void initVertexList();
private:
	static std::vector<Vertex> _imagePlaneVertices;

	Color _rayColor;
	std::unique_ptr<Vertex> _start;
	std::unique_ptr<Vertex> _end;
	std::unique_ptr<Vertex> _endTriangle;
};

class Triangle
{
public:
	Triangle(Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color);

	bool rayIntersection(Ray arg);
private:
	Vertex _v1, _v2, _v3;
	Direction _normal;
	Color _color;
};