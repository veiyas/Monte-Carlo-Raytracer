#include "photonmap.hpp"

PhotonMap::PhotonMap(std::vector<CeilingLight>& lights)
	: _map{}, _gen{ std::random_device{}() }, _rng{ 0.f, 1.f }
{
	std::cout << "Constructing photon map...   ";
	for (const auto& light : lights)
	{
		const auto lightCenterPoints = light.getCenterPoints();
		const float xCenter = lightCenterPoints.first - 0.5f;
		const float yCenter = lightCenterPoints.second - 0.5f;

		std::vector<PhotonNode> photonData;
		photonData.reserve(N_PHOTONS_TO_CAST);
		//Cast photons into scene ...
		for (size_t i = 0; i < N_PHOTONS_TO_CAST; i++)
		{
			const float randAzim = TWO_PI * _rng(_gen);
			const float randIncl = TWO_PI/4.f * _rng(_gen);
			const Vertex randPointOnLight{ xCenter + _rng(_gen), yCenter + _rng(_gen), 5.f, 1.f };
			const Vertex randEndPoint{
				randPointOnLight.x + glm::cos(randAzim) * glm::sin(randIncl),
				randPointOnLight.y + glm::sin(randAzim) * glm::sin(randIncl),
				randPointOnLight.z - glm::cos(randIncl),
				1.f
			};
			Ray photonRay{ randPointOnLight, randEndPoint };
		}

		
		
		//Insert photons into k-d tree with automatic balancing
		_map.efficient_replace_and_optimise(photonData);
	}
	std::cout << "done!\n";
}
