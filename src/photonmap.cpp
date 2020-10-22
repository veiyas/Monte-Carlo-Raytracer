#include "photonmap.hpp"

PhotonMap::PhotonMap(const SceneGeometry& geometry)
	: _map{}, _gen{ std::random_device{}() }, _rng{ 0.f, 1.f }
{
	auto startTime = std::chrono::high_resolution_clock::now();
	std::cout << "Constructing photon map...   ";

	std::vector<CeilingLight> lights = geometry._ceilingLights;
	for (const auto& light : lights)
	{
		const auto lightCenterPoints = light.getCenterPoints();
		const float xCenter = lightCenterPoints.first - 0.5f;
		const float yCenter = lightCenterPoints.second - 0.5f;

		std::vector<PhotonNode> photonData;
		photonData.reserve(N_PHOTONS_TO_CAST);

		for (size_t i = 0; i < N_PHOTONS_TO_CAST; i++)
		{			
			std::queue<Ray*> photonQueue;
			Photon initialPhoton = generateRandomPhotonFromLight(xCenter, yCenter);
			photonQueue.push(&initialPhoton);

			Photon* currentPhoton = nullptr;
			while(!photonQueue.empty())
			{
				std::vector<IntersectionSurface> photonIntersections;
				currentPhoton = photonQueue.front();
				photonQueue.pop();

				photonIntersection(*currentPhoton, geometry, photonIntersections);

				//One intersection, should only happen with diffuse surfaces
				if (photonIntersections.size() == 1 && photonIntersections[0].second == BRDF::DIFFUSE)
					photonData.push_back(PhotonNode{
							photonIntersections[0].first._intersectPoint,
							currentPhoton->getNormalizedDirection(),
							false });
				else if (photonIntersections.size() > 1) //Multiple intersections
				{
					//First hit is diffuse
					if (photonIntersections[0].second == BRDF::DIFFUSE)
					{
						photonData.push_back(PhotonNode{
							photonIntersections[0].first._intersectPoint,
							currentPhoton->getNormalizedDirection(),
							false });

						addShadowPhotons(photonIntersections, photonData, currentPhoton->getNormalizedDirection());
						//Do monte carlo stuff ...
					}
					else if (photonIntersections[0].second == BRDF::REFLECTOR)
					{
						//Compute refracted photon and add to queue
						const IntersectionData tempInter = photonIntersections[0].first;
						Ray reflectedPhoton = computeReflectedRay(tempInter._normal, *currentPhoton, tempInter._intersectPoint);
						photonQueue.push(&reflectedPhoton);

						addShadowPhotons(photonIntersections, photonData, currentPhoton->getNormalizedDirection());
					}
					else if (photonIntersections[0].second == BRDF::TRANSPARENT)
					{

					}
				}
			}
		}		
		//Insert photons into k-d tree with automatic balancing
		_map.efficient_replace_and_optimise(photonData);
	}
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = endTime - startTime;
	std::cout << "done!\nPhoton with of size " << _map.size() << " constructed in " << duration.count() << " seconds.\n\n";
}

void PhotonMap::addShadowPhotons(
	std::vector<IntersectionSurface>& inputData,
	std::vector<PhotonNode>& photonMapData,
	Direction photonDir)
{
	for (size_t i = 1; i < inputData.size(); i++)
	{
		auto tempInter = inputData[i].first;
		photonMapData.push_back(PhotonNode{
			tempInter._intersectPoint,
			photonDir,
			true
			});
	}
}

Ray PhotonMap::generateRandomPhotonFromLight(const float x, const float y)
{
	const Vertex randPointOnLight{ x + _rng(_gen), y + _rng(_gen), 5.f, 1.f };
	const float randAzim = TWO_PI * _rng(_gen);
	const float randIncl = TWO_PI / 4.f * _rng(_gen);
	const Vertex randEndPoint{
		randPointOnLight.x + glm::cos(randAzim) * glm::sin(randIncl),
		randPointOnLight.y + glm::sin(randAzim) * glm::sin(randIncl),
		randPointOnLight.z - glm::cos(randIncl),
		1.f
	};
	return Ray{ randPointOnLight, randEndPoint };
}
