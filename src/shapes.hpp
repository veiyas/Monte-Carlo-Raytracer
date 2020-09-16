#pragma once

#include <vector>
#include <iostream>
#include "glm/geometric.hpp"

#include "basic_types.hpp"

class Tetrahedron
{
public:
	Tetrahedron(float radius, Color color, Vertex position);
	Color getColor() const;
	std::pair<float, Triangle> rayIntersection(Ray& arg) const;
private:
	std::vector<Triangle> _triangles;
};

//TODO duplicate code ????????
class Sphere
{
public:
	Sphere(float radius, Color color, Vertex position, float alpha);
	Color getColor() const { return _color; }
	std::pair<float, Triangle> rayIntersection(Ray& arg) const;
private:
	const Vertex _position;
	const float _radius;
	const Color _color;
	float _alpha; //TODO transparency stuff, put this in BRDF
};