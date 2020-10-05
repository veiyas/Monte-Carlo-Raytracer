#include "brdf.hpp"

BRDF::BRDF(unsigned surfaceType)
	:	_surfaceType{surfaceType}
{

}

double BRDF::computeOrenNayar(const Direction& incoming, const Direction& shadowRay, const Direction& normal) const
{
	//Direction incomingNormalDiff = incoming - normal;
	//Direction shadowRayNormalDiff = shadowRay - normal;

	////Azimuth == PHI, Inclination == THETA
	//double incomingNormalAzimuth = glm::atan(incomingNormalDiff.x / incomingNormalDiff.y);
	//double incomingNormalInclination = glm::atan(incomingNormalDiff.z / incomingNormalDiff.y);

	//double A = 1.0 - 0.5 * (_roughness / (_roughness + 0.33));
	//double B = 0.45 * (_roughness / (_roughness + 0.09));

	//double shadowRayNormalAzimuth = glm::atan(shadowRayNormalDiff.x, shadowRayNormalDiff.y);
	//double shadowRayNormalInclination = glm::atan(shadowRayNormalDiff.z / shadowRayNormalDiff.y);

	//double alpha = glm::max(incomingNormalAzimuth, shadowRayNormalAzimuth);
	//double beta = glm::min(incomingNormalInclination, shadowRayNormalInclination);

	//double firstPart = (_albedo / glm::pi<double>());
	//double parenthesis =
	//	A + (B * glm::max(0.0, glm::cos(shadowRayNormalAzimuth - incomingNormalAzimuth)) * glm::sin(alpha) * glm::sin(beta));

	//return firstPart * parenthesis;

	//My code above doesnt work and i dont know why
	//This code below works and i dont know why
	//Lets not dwell on it, borrowed from https://github.com/kbladin/Monte_Carlo_Ray_Tracer/blob/master/src/Scene.cpp
	//TODO license???

	double A = 1.0 - 0.5 * (_roughness / (_roughness + 0.33));
	double B = 0.45 * (_roughness / (_roughness + 0.09));
	double cosThetaIncoming = glm::dot(incoming, normal);
	double cosThetaShadowRay = glm::dot(shadowRay, normal);
	double thetaShadowRay = glm::acos(cosThetaShadowRay);
	double thetaIncoming = glm::acos(cosThetaIncoming);
	double alpha = glm::max(thetaShadowRay, thetaIncoming);
	double beta = glm::min(thetaShadowRay, thetaIncoming);
	double cosIncomingShadowRay = glm::dot(incoming, shadowRay);

	return _albedo / glm::pi<double>() * (A + (B * glm::max(0.0, cosIncomingShadowRay)) * glm::sin(alpha) * glm::tan(beta));
}
