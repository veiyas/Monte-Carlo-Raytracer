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

void Tetrahedron::rayIntersections(Ray& ray, std::vector<IntersectionSurface>& toBeFilled) const
{
	for (auto& triangle : _triangles)
	{
		float t = triangle.rayIntersection(ray);
		if (t != -1)
		{
			IntersectionData temp{
				ray.getStart() + Vertex{ ray.getNormalizedDirection() * t, 0.0f },
				triangle.getNormal(),
				t };
			toBeFilled.push_back(std::make_pair(temp, getBRDF().getSurfaceType()));
		}
	}
}

Sphere::Sphere(BRDF brdf, float radius, Color color, Vertex position)
	: SceneObject{ brdf, color },
	_radius { radius }, _position{ position }
{

}

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

	if (expressionInSQRT < 0) //No intersections
		return {};
	else if (expressionInSQRT > -0 && expressionInSQRT < 0) //One intersection
		d = (-b) / 2;
	else // Two intersections
	{
		// Pick the intersection that is closest to the starting point of the ray, while givning a positive d
		d = ((-b) / 2) - glm::sqrt(expressionInSQRT);
		float otherPossibleD = ((-b) / 2) + glm::sqrt(expressionInSQRT);
		if (d < 0 && otherPossibleD > 0) // The intersecting ray is coming from inside the object
		{
			d = otherPossibleD;
			isInside = true;
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

	if (d < 0) // Intersection located behind the object
		return {};

	return IntersectionData{
		arg.getStart() + Vertex{ arg.getNormalizedDirection() * (float)d, 0.0f },
		intersectionPointNormal,
		(float)d
	};

}

void Sphere::rayIntersections(Ray& arg, std::vector<IntersectionSurface>& toBeFilled) const
{
	static constexpr float EPSILON = 0.00001;
	glm::vec3 rayStart{ arg.getStart().x, arg.getStart().y, arg.getStart().z };
	glm::vec3 rayEnd{ arg.getEnd().x, arg.getEnd().y, arg.getEnd().z };
	glm::vec3 rayDirectionNormalized = glm::normalize(rayEnd - rayStart);

	glm::vec3 o_c = rayStart - glm::vec3{ _position.x, _position.y, _position.z };

	double a = glm::dot(rayDirectionNormalized, rayDirectionNormalized);
	double b = glm::dot(o_c, rayDirectionNormalized * 2.0f);
	double c = glm::dot(o_c, o_c) - _radius * _radius;
	float d{};

	bool isInside = false;

	double expressionInSQRT = glm::pow(b / 2, 2) - a * c;

	if (expressionInSQRT < 0) //No intersections
		return;
	else if (expressionInSQRT > -EPSILON && expressionInSQRT < EPSILON) //One intersection
	{
		d = (-b) / 2.f;
		glm::vec3 intersection = rayStart + rayDirectionNormalized * d;
		glm::vec3 intersectionPointNormal = glm::normalize(glm::vec3{
			intersection.x - _position.x,
			intersection.y - _position.y,
			intersection.z - _position.z });
		IntersectionData temp{
			Vertex(intersection, 1.f),
			intersectionPointNormal,
			d
		};
		toBeFilled.push_back(std::make_pair(temp, getBRDF().getSurfaceType()));
	}
	else // Two intersections
	{
		d = ((-b) / 2) - glm::sqrt(expressionInSQRT);
		glm::vec3 intersection1 = rayStart + rayDirectionNormalized * d;
		glm::vec3 intersectionPointNormal1 = glm::normalize(glm::vec3{
			intersection1.x - _position.x,
			intersection1.y - _position.y,
			intersection1.z - _position.z });
		IntersectionData temp1{
			Vertex(intersection1, 1.f),
			intersectionPointNormal1,
			d
		};

		float d2 = ((-b) / 2) + glm::sqrt(expressionInSQRT);
		glm::vec3 intersection2 = rayStart + rayDirectionNormalized * d2;
		glm::vec3 intersectionPointNormal2 = glm::normalize(glm::vec3{
			intersection2.x - _position.x,
			intersection2.y - _position.y,
			intersection2.z - _position.z });
		IntersectionData temp2{
			Vertex(intersection2, 1.f),
			intersectionPointNormal2,
			d2
		};
		toBeFilled.push_back(std::make_pair(temp1, getBRDF().getSurfaceType()));
		toBeFilled.push_back(std::make_pair(temp2, getBRDF().getSurfaceType()));
	}

	//glm::vec3 intersection = rayStart + rayDirectionNormalized.operator*=(d);
	//glm::vec3 intersectionPointNormal = glm::normalize(glm::vec3{
	//	intersection.x - _position.x,
	//	intersection.y - _position.y,
	//	intersection.z - _position.z });

	//// Flip normal if intersecting from inside object
	//if (isInside)
	//	intersectionPointNormal *= -1.0f;
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

CeilingLight::CeilingLight(BRDF brdf, float xPos, float yPos)
	: SceneObject(brdf, WHITE_COLOR),
	  leftFar{ xPos + 0.5, yPos + 0.5f, 4.999f, 1.f },
	  leftClose{ xPos - 0.5, yPos + 0.5f, 4.999f, 1.f },
	  rightFar{ xPos + 0.5, yPos - 0.5f, 4.999f, 1.f },
	  rightClose{ xPos - 0.5, yPos - 0.5f, 4.999f, 1.f },
	  _centerPoints{std::make_pair(xPos, yPos)}
{
	TriangleObj t1{ brdf, leftClose, leftFar, rightFar, Direction(0, 0, -1), WHITE_COLOR };
	TriangleObj t2{ brdf, leftClose, rightFar, rightClose, Direction(0, 0, -1), WHITE_COLOR };
	_triangles.push_back(t1);
	_triangles.push_back(t2);
}

std::optional<IntersectionData> CeilingLight::rayIntersection(Ray& arg) const
{
	//We know there are always 2 triangles in the ceiling light
	auto intersection1 = _triangles[0].rayIntersection(arg);
	auto intersection2 = _triangles[1].rayIntersection(arg);

	if (intersection1 && !intersection2)
		return intersection1;
	else if (!intersection1 && intersection2)
		return intersection2;
	else
		return intersection1->_t < intersection2->_t ? intersection1 : intersection2;

	return {}; //No intersections
}
