#include "basic_types.hpp"
std::vector<Vertex> Ray::_imagePlaneVertices;

Ray::Ray(Vertex start, Vertex end)
{
	_imagePlaneVertices.push_back(std::move(start));
	_imagePlaneVertices.push_back(std::move(end));

	_start = std::make_unique<Vertex>(start);
	_end = std::make_unique<Vertex>(end);
}

void Ray::initVertexList()
{
	_imagePlaneVertices.reserve(72);
}

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color)
	: _v1{ v1 }, _v2{ v2 }, _v3{ v3 }, _normal{ normal }, _color{ color }
{

}

bool Triangle::rayIntersection(Ray& arg) const
{
	//TODO implement rayIntersection()
	return false;
}
