#pragma once

#include <iostream>
#include <optional>

#include <glm/gtx/vector_angle.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>

#include "basic_types.hpp"

class BRDF
{
public:
	BRDF(unsigned surfaceType, bool isLambertian = false);
	enum {
		DIFFUSE,
		TRANSPARENT,
		REFLECTOR,
		LIGHT
	};
	unsigned getSurfaceType() const { return _surfaceType; }
	double computeBRDF(const Direction& incoming, const Direction& shadowRay, const Direction& normal) const;

private:
	const unsigned _surfaceType;

	//https://en.wikipedia.org/wiki/Oren-Nayar_reflectance_model
	//These are only used for diffuse surfaces
	bool _isLambertian;
	constexpr static double _roughnessSquared = 0.5 * 0.5; //Variance == roughness
	constexpr static double _albedo = 0.9; //How much light is reflected, 1-albedo is absorbed

	double computeOrenNayar(const Direction& incoming, const Direction& shadowRay, const Direction& normal) const;
	double computeLambertian() const;
};