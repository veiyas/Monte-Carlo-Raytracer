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
	//For easier accesses
	friend class RayTree;

	Ray(Vertex start, Vertex end);
	Ray(Ray& ray);
	Ray(Ray&& ray) = default;

	static void initVertexList();

	void setLeft(Ray&& ray) { _left = std::make_unique<Ray>(ray); }
	std::unique_ptr<Ray>& getLeft() { return _left; }
	void setRight(Ray&& ray) { _right = std::make_unique<Ray>(ray); }
	std::unique_ptr<Ray>& getRight() { return _right; }

	Vertex getStart() const { return *_start; }
	Vertex getEnd() const { return *_end; }
	bool isInsideObject() const { return _isInsideObject; }
	Color getColor() const { return _rayColor; }
	void setColor(const Color color) { _rayColor = color; }
private:
	std::unique_ptr<Vertex> _end;
	std::unique_ptr<Vertex> _start;
	static std::vector<Vertex> _imagePlaneVertices; //?????

	bool _isInsideObject = false;

	//Left: reflected, Right: refracted
	std::unique_ptr<Ray> _left;
	std::unique_ptr<Ray> _right;

	Color _rayColor;
	std::unique_ptr<Vertex> _endTriangle;
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
	Triangle(Vertex v1, Vertex v2, Vertex v3, Color color); //Calculate normal
	Triangle(Vertex v, Direction normal, const Color& color); //Used for spheres with no triangles
	Triangle() = default;
	Color getColor() const { return _color; }
	Direction getNormal() const { return _normal; }
	Vertex getCenter() const;
	Vertex getPoint() const { return _v1; };
	float rayIntersection(Ray& arg) const;
private:
	Vertex _v1, _v2, _v3;
	Direction _normal;
	Color _color;
};