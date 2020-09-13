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

Sphere::Sphere(float radius, Color color, Vertex position, float alpha)
	: _radius{radius}, _color{color}, _position{position}, _alpha{ alpha }
{

}

std::pair<float, Color> Sphere::rayIntersection(Ray& arg) const
{
	Vertex directionVertex = arg.getEnd() - arg.getStart();
	//rayDirectionNorm = I
	Direction rayDirectionNorm =
		glm::normalize(Direction{directionVertex.x, directionVertex.y, directionVertex.z });
	double a = glm::dot(rayDirectionNorm, rayDirectionNorm);

	//Recycle rayDirection norm as 2I
	rayDirectionNorm *= 2;
	Vertex ocVertex = arg.getStart() - _position;
	Direction o_c{ ocVertex.x, ocVertex.y, ocVertex.z };
	double b = glm::dot(rayDirectionNorm, o_c);
	double c = glm::dot(o_c, o_c) - _radius;

	double sqrtExpression = glm::pow((b / 2), 2) - a * c;

	if (sqrtExpression < 0) //No intersection
		return std::make_pair(-1, Color{ });
	else if (sqrtExpression < 1e-6 && sqrtExpression > -1e-6) //One intersection
		return std::make_pair(-b / 2, getColor());

	//float dPlus = (-b / 2) + glm::sqrt(glm::pow((b / 2), 2) - a * c);
	float dMinus = (-b / 2) - glm::sqrt(sqrtExpression);
	return std::make_pair(dMinus, getColor());
}
