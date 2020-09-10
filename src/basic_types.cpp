#include "basic_types.hpp"

std::vector<Vertex> Ray::_vertexList;

Ray::Ray(Vertex start, Vertex end)
{
	_vertexList.push_back(start);
	_vertexList.push_back(end);

	_start = std::make_unique<Vertex>(std::move(start));
	_end = std::make_unique<Vertex>(std::move(end));
}

void Ray::initVertexList()
{
	//Scene: 24 triangles = 72 vertices
	_vertexList.reserve(72);
}
