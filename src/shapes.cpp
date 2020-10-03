#include "shapes.hpp"

#include <glm/gtx/rotate_vector.hpp>

#include "ray.hpp"


Tetrahedron::Tetrahedron(BRDF brdf, float radius, Color color, Vertex position)
	: SceneObject{ brdf, color }
{
	_triangles.reserve(4);

	// Yes its ugly hehe
	const Vertex v[4] = {                            // zeros to keep w=0 v
		radius * glm::rotateX(glm::rotateZ(Vertex{  1.0f,  1.0f,  1.0f, 0.0f }, 1.0f), 1.0f) + position,
		radius * glm::rotateX(glm::rotateZ(Vertex{ -1.0f, -1.0f,  1.0f, 0.0f }, 1.0f), 1.0f) + position,
		radius * glm::rotateX(glm::rotateZ(Vertex{ -1.0f,  1.0f, -1.0f, 0.0f }, 1.0f), 1.0f) + position,
		radius * glm::rotateX(glm::rotateZ(Vertex{  1.0f, -1.0f, -1.0f, 0.0f }, 1.0f), 1.0f) + position
	};


	_triangles.emplace_back(v[2], v[1], v[0], color);
	_triangles.emplace_back(v[0], v[3], v[2], color);
	_triangles.emplace_back(v[1], v[3], v[0], color);
	_triangles.emplace_back(v[2], v[3], v[1], color);
}

std::optional<IntersectionData> Tetrahedron::rayIntersection(Ray& ray) const
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
	{
		Direction intersectNormal = closestIntersectingTriangle->getNormal();
		// If intersected from inside, the normal is flipped
		if (glm::dot(ray.getNormalizedDirection(), intersectNormal) > 0)
		{
			intersectNormal *= -1.0f;
		}

		return IntersectionData{
			ray.getStart() + Vertex{ ray.getNormalizedDirection() * minT, 0.0f },
			intersectNormal,
			minT
		};
	}

	return {};
}

Sphere::Sphere(BRDF brdf, float radius, Color color, Vertex position)
	: SceneObject{ brdf, color },
	_radius { radius }, _position{ position }
{

}

static constexpr double EPSILON = 1e-4;
std::optional<IntersectionData> Sphere::rayIntersection(Ray& arg) const
{
	glm::vec3 rayStart{ arg.getStart().x, arg.getStart().y, arg.getStart().z };
	glm::vec3 rayEnd{ arg.getEnd().x, arg.getEnd().y, arg.getEnd().z };
	glm::vec3 rayDirectionNormalized = glm::normalize(rayEnd - rayStart);

	glm::vec3 o_c = rayStart - glm::vec3{ _position.x, _position.y, _position.z };

	double a = glm::dot(rayDirectionNormalized, rayDirectionNormalized);
	double b = glm::dot(o_c, rayDirectionNormalized * 2.0f);
	double c = glm::dot(o_c, o_c) - _radius * _radius;
	double d{};

	bool isInside = false;

	double expressionInSQRT = glm::pow(b / 2, 2) - a * c;

	if (expressionInSQRT < -EPSILON) //No intersections
		return {};
	else if (expressionInSQRT > -EPSILON && expressionInSQRT < EPSILON) //One intersection
		d = (-b) / 2;
	else // Two intersections
	{
		// Pick the intersection that is closest to the starting point of the ray, while givning a positive d
		d = ((-b) / 2) - glm::sqrt(expressionInSQRT);
		float otherPossibleD = ((-b) / 2) + glm::sqrt(expressionInSQRT);
		if (d < EPSILON && otherPossibleD > EPSILON) // The intersecting ray is coming from inside the object
		{
			d = otherPossibleD;
			isInside = true;
			//std::cout << "kjkl\n";
		}
	}

	glm::vec3 intersection = rayStart + rayDirectionNormalized.operator*=(d);
	glm::vec3 intersectionPointNormal = glm::normalize(glm::vec3{
		intersection.x - _position.x,
		intersection.y - _position.y,
		intersection.z - _position.z });

	// Flip normal if intersecting from inside object
	if (isInside)
		intersectionPointNormal *= -1.0f;

	//std::cout << rayEnd.x + rayDirectionNormalized.x*d << "\n";

	if (d < EPSILON) // Intersection located behind the object
		return {};

	return IntersectionData{
		arg.getStart() + Vertex{ arg.getNormalizedDirection() * (float)d, 0.0f },
		intersectionPointNormal,
		(float)d
	};

}

TriangleObj::TriangleObj(BRDF brdf, Vertex v1, Vertex v2, Vertex v3, Color color)
	: SceneObject{ brdf, color }, _basicTriangle{v1, v2, v3, color }
{ }

TriangleObj::TriangleObj(BRDF brdf, Vertex v1, Vertex v2, Vertex v3, Direction normal, Color color)
	: SceneObject{ brdf, color }, _basicTriangle{ v1, v2, v3, normal, color }
{ }

std::optional<IntersectionData> TriangleObj::rayIntersection(Ray& arg) const
{
	float t = _basicTriangle.rayIntersection(arg);
	if (t == -1)
		return {};

	return IntersectionData{
		arg.getStart() + Vertex{ arg.getNormalizedDirection() * t, 0.0f },
		_basicTriangle.getNormal(),
		t
	};
}
