#include "photonmap.hpp"

PhotonMap::PhotonMap(const SceneGeometry& geometry)
	: _photonMap{}, _gen{ std::random_device{}() }, _rng{ 0.f, 1.f }, _deltaFlux{ calculateDeltaFlux() }
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
		photonData.reserve(N_PHOTONS_TO_CAST*10u);

		for (size_t i = 0; i < N_PHOTONS_TO_CAST; i++)
		{			
			std::queue<Photon> photonQueue;

			Photon initialPhoton = generateRandomPhotonFromLight(xCenter, yCenter);
			photonQueue.push(std::move(initialPhoton));

			while(!photonQueue.empty())
			{
				std::vector<IntersectionSurface> photonIntersections;
				Photon currentPhoton = std::move(photonQueue.front());
				photonQueue.pop();

				photonIntersection(currentPhoton, geometry, photonIntersections);

				//One intersection, should only happen with diffuse surfaces
				if (photonIntersections.size() == 1 && photonIntersections[0].second == BRDF::DIFFUSE)
				{
					photonData.push_back(PhotonNode{
							photonIntersections[0].first._intersectPoint,
							_deltaFlux * static_cast<float>(currentPhoton.getColor().x),
							currentPhoton.getNormalizedDirection() });
					handleMonteCarloPhoton(photonQueue, photonIntersections[0].first, currentPhoton);
				}
				else if (photonIntersections.size() > 1) //Multiple intersections
				{
					//First hit is diffuse
					if (photonIntersections[0].second == BRDF::DIFFUSE)
					{
						photonData.push_back(PhotonNode{
							photonIntersections[0].first._intersectPoint,
							_deltaFlux * static_cast<float>(currentPhoton.getColor().x),
							currentPhoton.getNormalizedDirection()});

						addShadowPhotons(photonIntersections);
						handleMonteCarloPhoton(photonQueue, photonIntersections[0].first, currentPhoton);
					}
					else if (photonIntersections[0].second == BRDF::REFLECTOR)
					{
						//Compute refracted photon and add to queue
						const IntersectionData tempInter = photonIntersections[0].first;
						Photon reflectedPhoton = computeReflectedRay(tempInter._normal, currentPhoton, tempInter._intersectPoint);
						photonQueue.push(std::move(reflectedPhoton));

						addShadowPhotons(photonIntersections);
					}
					else if (photonIntersections[0].second == BRDF::TRANSPARENT)
					{
						const IntersectionData tempInter = photonIntersections[0].first;
						float incAngle = glm::angle(-currentPhoton.getNormalizedDirection(), photonIntersections[0].first._normal);
						double reflectionCoeff, n1, n2;
						bool rayIsTransmitted = shouldRayTransmit(n1, n2, reflectionCoeff, incAngle, currentPhoton);

						if (rayIsTransmitted)
						{
							Photon refractedPhoton = computeRefractedRay(
								tempInter._normal, currentPhoton, tempInter._intersectPoint, currentPhoton.isInsideObject());
							refractedPhoton.setColor(refractedPhoton.getColor() * (1.f - reflectionCoeff));

							//THIS CREATES AN INFINITE LOOP for some reason
							//photonQueue.push(std::move(refractedPhoton));
						}
						Photon reflectedPhoton = computeReflectedRay(tempInter._normal, currentPhoton, tempInter._intersectPoint);
						reflectedPhoton.setColor(reflectedPhoton.getColor() * reflectionCoeff);
						photonQueue.push(std::move(reflectedPhoton));
					}
				}
			}
		}
		_photonMap.efficient_replace_and_optimise(photonData);
	}
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = endTime - startTime;
	std::cout << "done!\nPhoton map with " << _photonMap.size() << " photons constructed in " << duration.count() << " seconds.\n"
		<< static_cast<long long int>(_photonMap.size() - N_PHOTONS_TO_CAST) << " extra photons were created from specular surfaces.\n\n";
}

void PhotonMap::getPhotons(std::vector<PhotonNode>& foundPhotons, const PhotonNode& searchPoint)
{
	_photonMap.find_within_range(searchPoint, SEARCH_RANGE, std::back_inserter(foundPhotons));
}

bool PhotonMap::shadowPhotonsPresent(const PhotonNode& searchPoint)
{
	return _shadowPhotonMap.count_within_range(searchPoint, SEARCH_RANGE) > 0 ? true : false;
}

void PhotonMap::addShadowPhotons(std::vector<IntersectionSurface>& inputData)
{
	for (size_t i = 1; i < inputData.size(); i++)
	{
		auto tempInter = inputData[i].first;
		_shadowPhotonMap.insert((PhotonNode{tempInter._intersectPoint}));
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

float PhotonMap::calculateDeltaFlux() const
{
	//The radiant flux is 1000 W, solid angle is pi (light emitted in hemisphere), area = 1
	//Therefore the radiance L0 is flux / (steradian * area) = 1000 / pi = 318.30989
	//From lecture 7 slide 10
	// 0.5pi * L0 = 500.000...

	return 500.f / static_cast<float>(N_PHOTONS_TO_CAST);
}

void PhotonMap::handleMonteCarloPhoton(std::queue<Ray>& queue, IntersectionData& inter, Photon& currentPhoton)
{
	if (_rng(_gen) + _terminationProbability > 1.f)
	{
		Photon generatedPhoton = generateRandomReflectedRay(
			currentPhoton.getNormalizedDirection(),
			inter._normal,
			inter._intersectPoint,
			_rng(_gen),
			_rng(_gen));
		queue.push(std::move(generatedPhoton));
	}
}
