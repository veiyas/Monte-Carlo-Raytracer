#include "ray.hpp"

#include <glm/glm.hpp>

Ray::Ray(Vertex start, Vertex end)
	: _rayColor{ Color{ 1.0, 1.0, 1.0 } }
{
	_start = std::make_unique<Vertex>(start);
	_end = std::make_unique<Vertex>(end);
}

Ray::Ray(Vertex start, Vertex end, Color color)
	: Ray{ start, end }
{
	_rayColor = color;
}

Ray::Ray(Ray& ray)
{
	// TODO Using std::move when making a copy seems weird, is it correct?
	// It really can't be. I have no idea why this was ever done
	_left = std::move(ray._left);
	_right = std::move(ray._right);
	_start = std::move(ray._start);
	_end = std::move(ray._end);

	_rayColor = ray._rayColor;
	_isInsideObject = ray._isInsideObject;
}

Direction Ray::getNormalizedDirection() const
{
	// TODO Consider storing the direction as a member
	return glm::normalize(Direction{ getEnd() } - Direction{ getStart() });
}
