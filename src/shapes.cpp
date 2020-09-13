#include "shapes.hpp"

Tetrahedron::Tetrahedron(float radius, Color color, Vertex position)
{
	_triangles.reserve(4);

	const Vertex v[4] = {
		radius * Vertex{  1.0f,  1.0f,  1.0f, 1.0f } +position,
		radius * Vertex{ -1.0f, -1.0f,  1.0f, 1.0f } +position,
		radius * Vertex{ -1.0f,  1.0f, -1.0f, 1.0f } +position,
		radius * Vertex{  1.0f, -1.0f, -1.0f, 1.0f } +position
	};

	// TODO Fix direction -- maybe do automatically in triangle
	_triangles.emplace_back(v[2], v[1], v[0], Direction{ }, color);
	_triangles.emplace_back(v[0], v[3], v[2], Direction{ }, color);
	_triangles.emplace_back(v[1], v[3], v[0], Direction{ }, color);
	_triangles.emplace_back(v[2], v[3], v[1], Direction{ }, color);
}

Color Tetrahedron::getColor() const
{
	return _triangles[0].getColor();
}

std::pair<float, Color> Tetrahedron::rayIntersection(Ray& ray) const
{
	const Triangle* closestIntersectingTriangle = nullptr;
	float minT = 1e+10;
	for (auto& triangle : _triangles)
	{
		float t = triangle.rayIntersection(ray);
		if (t != -1 && t < minT)
		{
			closestIntersectingTriangle = &triangle;
			minT = t;
		}
	}

	if (closestIntersectingTriangle != nullptr)
		return std::make_pair(minT, closestIntersectingTriangle->getColor());
	else
		return std::make_pair(-1, Color{ });
}