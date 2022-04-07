#pragma once

#include "basic_types.hpp"

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
