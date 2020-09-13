#pragma once

#include <vector>
#include <iostream>
#include <array>

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