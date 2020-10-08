#pragma once

#include <iostream>
#include <optional>

#include <glm/gtx/vector_angle.hpp>
#include <glm/geometric.hpp>

#include "basic_types.hpp"

class BRDF
{
public:
	BRDF(unsigned surfaceType);
	enum {
		DIFFUSE,
		TRANSPARENT,
		REFLECTOR,
		LIGHT
	};
	unsigned getSurfaceType() const { return _surfaceType; }
	double computeOrenNayar(const Direction& incoming, const Direction& shadowRay, const Direction& normal) const;

private:
	const unsigned _surfaceType;

	//https://en.wikipedia.org/wiki/Oren-Nayar_reflectance_model
	constexpr static double _roughnessSquared = 0.5 * 0.5; //Variance == roughness
	constexpr static double _albedo = 0.9; //How much light is reflected, 1-albedo is absorbed
	
};