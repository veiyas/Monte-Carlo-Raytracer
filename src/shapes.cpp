#include "shapes.hpp"

Tetrahedron::Tetrahedron(BRDF brdf, float radius, Color color, Vertex position)
	: BasicProperties{ brdf, color }
{
	_triangles.reserve(4);

	const Vertex v[4] = {
		radius * Vertex{  1.0f,  1.0f,  1.0f, 1.0f } +position,
		radius * Vertex{ -1.0f, -1.0f,  1.0f, 1.0f } +position,
		radius * Vertex{ -1.0f,  1.0f, -1.0f, 1.0f } +position,
		radius * Vertex{  1.0f, -1.0f, -1.0f, 1.0f } +position
	};

	// TODO Fix direction -- maybe do automatically in triangle
	_triangles.emplace_back(v[2], v[1], v[0], color);
	_triangles.emplace_back(v[0], v[3], v[2], color);
	_triangles.emplace_back(v[1], v[3], v[0], color);
	_triangles.emplace_back(v[2], v[3], v[1], color);
}

std::pair<float, Triangle> Tetrahedron::rayIntersection(Ray& ray) const
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
		return std::make_pair(minT, *closestIntersectingTriangle);
	else
		return std::make_pair(-1, Triangle{});
}

Sphere::Sphere(BRDF brdf, float radius, Color color, Vertex position)
	: BasicProperties{ brdf, color },
	_radius { radius }, _position{ position }
{

}

double EPSILON = 1e-4;
std::pair<float, Triangle> Sphere::rayIntersection(Ray& arg) const
{
	glm::vec3 rayStart{ arg.getStart().x, arg.getStart().y, arg.getStart().z };
	glm::vec3 rayEnd{ arg.getEnd().x, arg.getEnd().y, arg.getEnd().z };
	glm::vec3 rayDirectionNormalized = glm::normalize(rayEnd - rayStart);

	glm::vec3 o_c = rayStart - glm::vec3{ _position.x, _position.y, _position.z };

	double a = glm::dot(rayDirectionNormalized, rayDirectionNormalized);
	double b = glm::dot(o_c, rayDirectionNormalized.operator*=(2));
	double c = glm::dot(o_c, o_c) - _radius * _radius;
	double d{};

	rayDirectionNormalized /= 2; //Reset to normalized vector
	double expressionInSQRT = glm::pow(b / 2, 2) - a * c;

	if (expressionInSQRT < -EPSILON) //No intersection
		return std::make_pair(-1, Triangle());
	else if (expressionInSQRT > -EPSILON && expressionInSQRT < EPSILON) //One intersection
		d = (-b) / 2;
	else
		d = ((-b) / 2) - glm::sqrt(expressionInSQRT);

	glm::vec3 intersection = rayStart + rayDirectionNormalized.operator*=(d);
	glm::vec3 intersectionPointNormal = glm::normalize(glm::vec3{
		intersection.x - _position.x,
		intersection.y - _position.y,
		intersection.z - _position.z });

	//std::cout << rayEnd.x + rayDirectionNormalized.x*d << "\n";

	if (d < EPSILON) // Intersection located behind the object
		return std::make_pair(-1, Triangle());

	return std::make_pair(d, Triangle(glm::vec4(intersection, 1.0), intersectionPointNormal, getColor()));

}
