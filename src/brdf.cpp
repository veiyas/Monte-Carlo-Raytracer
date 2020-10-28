#include "brdf.hpp"

BRDF::BRDF(unsigned surfaceType)
	:	_surfaceType{surfaceType}
{

}

double BRDF::computeOrenNayar(const Direction& incoming, const Direction& shadowRay, const Direction& normal) const
{

	//Borrowed from https://github.com/kbladin/Monte_Carlo_Ray_Tracer/blob/master/src/Scene.cpp
	//TODO license

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
	
	//if (res > 1e10)
	//{
	//	std::cout << "A: " << A << "\n";
	//	std::cout << "B: " << B << "\n";
	//	std::cout << "cosThetaIncoming: " << cosThetaIncoming << "\n";
	//	std::cout << "cosThetaShadowRay: " << cosThetaShadowRay << "\n";
	//	std::cout << "thetaShadowRay: " << thetaShadowRay << "\n";
	//	std::cout << "thetaIncoming: " << thetaIncoming << "\n";
	//	std::cout << "alpha: " << alpha << "\n";
	//	std::cout << "beta: " << beta << "\n";
	//	std::cout << "cosIncomingShadowRay: " << cosIncomingShadowRay << "\n";
	//	std::cout << "res: " << res << "\n\n";

	//	//std::cout << "Incoming: " << glm::to_string(incoming) << "\n";
	//	//std::cout << "shadowRay: " << glm::to_string(shadowRay) << "\n";
	//	//std::cout << "normal: " << glm::to_string(normal) << "\n\n";
	//}

	return glm::min(res, 1.0);
}
