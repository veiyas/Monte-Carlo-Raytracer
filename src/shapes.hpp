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

	//virtual std::optional<IntersectionData> rayIntersection(Ray& arg) const = 0;
	const BRDF& accessBRDF() const { return _brdf; }
	BRDF getBRDF() const { return _brdf; }
	Color getColor() const { return _color; }
private:
	const BRDF _brdf;
	Color _color;
};

class Tetrahedron : public SceneObject
{
public:
	Tetrahedron(BRDF brdf, float radius, Color color, Vertex position);
	std::optional<IntersectionData> rayIntersection(Ray& arg) const;
private:
	std::vector<Triangle> _triangles;
};

class Sphere : public SceneObject
{
public:
	Sphere(BRDF brdf, float radius, Color color, Vertex position);
	
	std::optional<IntersectionData> rayIntersection(Ray& arg) const;
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
private:
	const Triangle _basicTriangle;
};

class CeilingLight : public SceneObject
{
public:
	CeilingLight(BRDF brdf, float xPos, float yPos);
	std::optional<IntersectionData> rayIntersection(Ray& arg) const;

	std::pair<float, float> getCenterPoints() const { return _centerPoints; }
private:
	std::vector<TriangleObj> _triangles;
	std::pair<float, float> _centerPoints;
};

struct SceneGeometry
{
	std::vector<TriangleObj> _sceneTris;
	std::vector<Tetrahedron> _tetrahedons;
	std::vector<Sphere> _spheres;
	std::vector<CeilingLight> _ceilingLights;
};