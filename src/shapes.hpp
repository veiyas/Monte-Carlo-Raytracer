#pragma once

#include <vector>
#include <iostream>
#include <array>

#include "glm/geometric.hpp"

#include "basic_types.hpp"

class Tetrahedron
{
public:
	Tetrahedron(float radius, Color color, Vertex position);
	Color getColor() const;
	std::pair<float, Color> rayIntersection(Ray& arg) const;
private:
	std::vector<Triangle> _triangles;
};

//TODO duplicate code ????????
class Sphere
{
public:
	Sphere(float radius, Color color, Vertex position, float alpha);
	Color getColor() const { return _color; }
	std::pair<float, Color> rayIntersection(Ray& arg) const;
private:
	Vertex _position;
	float _radius;
	Color _color;
	float _alpha; //TODO transparency stuff
};