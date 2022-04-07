#pragma once
#define WHITE_COLOR Color(1.0, 1.0, 1.0)
#include <vector>
#include <iostream>
#include <optional>
#include <tuple>

#include "glm/geometric.hpp"

#include "basic_types.hpp"
#include "brdf.hpp"
#include "triangle.hpp"


class SceneObject
{
public:
	SceneObject(BRDF brdf, Color color)
		: _brdf{ std::move(brdf) }, _color{ color } {}

	const BRDF& accessBRDF() const { return _brdf; }
	BRDF getBRDF() const { return _brdf; }
	Color getColor() const { return _color; }
private:
	const BRDF _brdf;
	Color _color;
};

struct IntersectionSurface
{
	IntersectionData intersectionData;
	const SceneObject* intersectionObject;
};

class Tetrahedron : public SceneObject
{
public:
	Tetrahedron(BRDF brdf, float radius, Color color, Vertex position);
	std::optional<IntersectionData> rayIntersection(Ray& arg) const;
	void rayIntersections(Ray& ray, std::vector<IntersectionSurface>& toBeFilled) const;
private:
	std::vector<Triangle> _triangles;
};

class Sphere : public SceneObject
{
public:
	Sphere(BRDF brdf, float radius, Color color, Vertex position);
	
	std::optional<IntersectionData> rayIntersection(Ray& arg) const;
	void rayIntersections(Ray& arg, std::vector<IntersectionSurface>& toBeFilled) const;
private:
	const Vertex _position;
	const float _radius;
};

// This way triangles can be treated the same way as spheres
// and tetrahedrons, and they also have the basic properties
class TriangleObj : public SceneObject
{
public:
	TriangleObj() = default;
	TriangleObj(BRDF brdf, Vertex v1, Vertex v2, Vertex v3, Color color);
	TriangleObj(BRDF brdf, Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color);
	std::optional<IntersectionData> rayIntersection(Ray& arg) const;
	Direction getNormal() const { return _basicTriangle.getNormal(); }
private:
	const Triangle _basicTriangle;
};

class CeilingLight : public SceneObject
{
public:
	CeilingLight(BRDF brdf, float xPos, float yPos);
	std::optional<IntersectionData> rayIntersection(Ray& arg) const;
	Direction getNormal() const { return _triangles[0].getNormal(); }

	// Cache corner points for use with shadow rays
	const Vertex leftFar;
	const Vertex leftClose;
	const Vertex rightFar;
	const Vertex rightClose;

	std::pair<float, float> getCenterPoints() const { return _centerPoints; }
private:
	std::vector<TriangleObj> _triangles;
	std::pair<float, float> _centerPoints;
};
