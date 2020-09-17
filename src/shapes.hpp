#pragma once

#include <vector>
#include <iostream>
#include "glm/geometric.hpp"

#include "basic_types.hpp"
#include "brdf.hpp"

class BasicProperties
{
public:
	BasicProperties(BRDF brdf, Color color)
		: _brdf{ std::move(brdf) }, _color{ color } {}

	const BRDF& getBRDF() const { return _brdf; }
	Color getColor() const { return _color; }
private:
	const BRDF _brdf;
	const Color _color;
};

class Tetrahedron : private BasicProperties
{
public:
	Tetrahedron(BRDF brdf, float radius, Color color, Vertex position);
	std::pair<float, Triangle> rayIntersection(Ray& arg) const;
private:
	std::vector<Triangle> _triangles;
};

class Sphere : private BasicProperties
{
public:
	Sphere(BRDF brdf, float radius, Color color, Vertex position, float alpha);
	
	std::pair<float, Triangle> rayIntersection(Ray& arg) const;
private:
	const Vertex _position;
	const float _radius;
};