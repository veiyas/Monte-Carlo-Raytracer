#pragma once

#include "basic_types.hpp"
#include "triangle.hpp"

class Ray
{
public:
	Ray(Vertex start, Vertex end);
	Ray(Vertex start, Vertex end, Color color);
	Ray(Ray& ray);
	Ray(Ray&& ray) = default;

	static void initVertexList();

	void setLeft(Ray&& ray) { _left = std::make_unique<Ray>(ray); }
	Ray* getLeft() { return  _left ? _left.get() : nullptr; }
	void setRight(Ray&& ray) { _right = std::make_unique<Ray>(ray); }
	Ray* getRight() { return _right ? _right.get() : nullptr; }
	void setParent(Ray* ray) { _parent = ray; }
	Ray* getParent() { return _parent; }

	Vertex getStart() const { return *_start; }
	Vertex getEnd() const { return *_end; }
	Direction getNormalizedDirection() const;

	Triangle* getEndTriangle() const { return _endTriangle.get(); }
	void setEndTriangle(Triangle& tri) { _endTriangle = std::make_unique<Triangle>(tri); }

	void setInsideObject(bool isInside) { _isInsideObject = isInside; }
	bool isInsideObject() const { return _isInsideObject; }
	Color getColor() const { return _rayColor; }
	void setColor(const Color color) { _rayColor = color; }
	void setIntersectPoint(Vertex point) { _intersectionPoint = point; }
	const Vertex& getIntersectionPoint() const { return _intersectionPoint; }

	double getShadow() const { return _shadow; }
	void setShadow(double shadow) { _shadow = shadow; }
private:
	std::unique_ptr<Vertex> _end;
	std::unique_ptr<Vertex> _start;
	static std::vector<Vertex> _imagePlaneVertices; //?????

	bool _isInsideObject = false;
	double _shadow;

	Vertex _intersectionPoint;

	//Left: reflected, Right: refracted
	std::unique_ptr<Ray> _left;
	std::unique_ptr<Ray> _right;
	Ray* _parent = nullptr;

	// Used to carry importance and probably 
	// later radiance for photon mapping (i think)
	Color _rayColor;
	std::unique_ptr<Triangle> _endTriangle;
};