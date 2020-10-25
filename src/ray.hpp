#pragma once

#include "triangle.hpp"
#include "brdf.hpp"
#include "shapes.hpp"

class Ray
{
public:
	Ray(Vertex start, Vertex end);
	Ray(Vertex start, Vertex end, Color color);
	Ray(Ray& ray);
	Ray(Ray&& ray) = default;
	Ray& operator=(Ray&&) = default;

	void setLeft(Ray&& ray) { _left = std::make_unique<Ray>(ray); }
	Ray* getLeft() { return  _left ? _left.get() : nullptr; }
	void setRight(Ray&& ray) { _right = std::make_unique<Ray>(ray); }
	Ray* getRight() { return _right ? _right.get() : nullptr; }
	void setParent(Ray* ray) { _parent = ray; }
	Ray* getParent() { return _parent; }

	Vertex getStart() const { return *_start; }
	Vertex getEnd() const { return *_end; }
	Direction getNormalizedDirection() const;

	void setInsideObject(bool isInside) { _isInsideObject = isInside; }
	bool isInsideObject() const { return _isInsideObject; }

	// The color member is used to store importance when casting from camera
	Color getColor() const { return _rayColor; }
	void setColor(const Color color) { _rayColor = color; }

	// Maybe this could be optimized...
	void setIntersectionData(IntersectionData data) { _intersectionData = data; }
	const std::optional<IntersectionData>& getIntersectionData() const { return _intersectionData; }
	void setIntersectedObject(const SceneObject* obj) { _intersectedObject = obj; }
	const std::optional<const SceneObject*>& getIntersectedObject() const { return _intersectedObject; }

private:
	std::unique_ptr<Vertex> _end;
	std::unique_ptr<Vertex> _start;

	bool _isInsideObject = false;

	std::optional<IntersectionData> _intersectionData;
	std::optional<const SceneObject*> _intersectedObject;

	//Unique ptrs are used to avoid memory leaks
	//Left: reflected, Right: refracted
	std::unique_ptr<Ray> _left;
	std::unique_ptr<Ray> _right;
	Ray* _parent = nullptr;

	// Used to carry importance and probably 
	// later radiance for photon mapping (i think)
	Color _rayColor;
};