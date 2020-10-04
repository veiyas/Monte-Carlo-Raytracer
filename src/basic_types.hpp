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

class Ray;

class Pixel
{
public:
	void addRay(std::shared_ptr<Ray> ray) { _ray = ray; }
	Color _color{ };
private:
	std::shared_ptr<Ray> _ray; // The ray that goes through the pixel
};

struct IntersectionData
{
	IntersectionData(Vertex point, Direction normal, float t)
		: _intersectPoint{ point }, _normal{ normal }, _t{ t } {}
	Vertex _intersectPoint;
	Direction _normal;
	float _t;
};