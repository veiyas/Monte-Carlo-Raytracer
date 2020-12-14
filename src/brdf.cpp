#include "brdf.hpp"

BRDF::BRDF(unsigned surfaceType, bool isLambertian)
	:	_surfaceType{surfaceType}, _isLambertian{ isLambertian }
{

}

double BRDF::computeBRDF(const Direction& incoming, const Direction& shadowRay, const Direction& normal) const
{
	return _isLambertian ? computeLambertian() : computeOrenNayar(incoming, shadowRay, normal);
}

double BRDF::computeOrenNayar(const Direction& incoming, const Direction& shadowRay, const Direction& normal) const
{
	//Borrowed from: https://github.com/kbladin/Monte_Carlo_Ray_Tracer/blob/master/src/Scene.cpp
	//License: https://github.com/kbladin/Monte_Carlo_Ray_Tracer/blob/master/LICENSE

	double A = 1.0 - 0.5 * (_roughnessSquared / (_roughnessSquared + 0.33));
	double B = 0.45 * (_roughnessSquared / (_roughnessSquared + 0.09));
	double cosThetaIncoming = glm::dot(incoming, normal);
	// Min here is used to avoid numerical errors which cause -nan(ind)
	double cosThetaShadowRay = glm::min((double)glm::dot(shadowRay, normal), 1.0);
	double thetaShadowRay = glm::acos(cosThetaShadowRay);
	double thetaIncoming = glm::acos(cosThetaIncoming);
	double alpha = glm::max(thetaShadowRay, thetaIncoming);
	double beta = glm::min(thetaShadowRay, thetaIncoming);
	double cosIncomingShadowRay = glm::dot(incoming, shadowRay);

	double res = (_albedo / 3.1415) * (A + (B * glm::max(0.0, cosIncomingShadowRay)) * glm::sin(alpha) * glm::tan(beta));
	return glm::min(res, 1.0);
}

double BRDF::computeLambertian() const
{
	return _albedo / glm::pi<double>();
}
