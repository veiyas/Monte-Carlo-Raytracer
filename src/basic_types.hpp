#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <iostream>
#include <array>

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
	Vertex getStart() const { return *_start; }
	Vertex getEnd() const { return *_end; }
private:
	static std::vector<Vertex> _imagePlaneVertices;

	Color _rayColor;
	std::unique_ptr<Vertex> _start;
	std::unique_ptr<Vertex> _end;
	std::unique_ptr<Vertex> _endTriangle;

	std::mutex arrayLock;
};

class Pixel
{
public:
	void addRay(std::shared_ptr<Ray> ray) { _ray = ray; }
	Color _color{ };
private:
	std::shared_ptr<Ray> _ray; // The ray that goes through the pixel
};

class Triangle
{
public:
	Triangle(Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color);
	Color getColor() const { return _color; }
	float rayIntersection(Ray& arg) const;
private:
	Vertex _v1, _v2, _v3;
	Direction _normal;
	Color _color;
};

class Tetrahedron
{
public:
	Tetrahedron(float radius, Color color, Vertex position);
	Color getColor() const;
	std::pair<float, Color> rayIntersection(Ray& arg) const;
private:
	std::vector<Triangle> _triangles;
};