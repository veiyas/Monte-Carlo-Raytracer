#include "photonmap.hpp"

PhotonMap::PhotonMap(const SceneGeometry& geometry)
	: _photonMap{}, _gen{ std::random_device{}() }, _rng{ 0.f, 1.f }, _deltaFlux{ calculateDeltaFlux() }
{
	std::cout << "Constructing photon map...   ";
	auto startTime = std::chrono::high_resolution_clock::now();

	std::vector<CeilingLight> lights = geometry._ceilingLights;
	for (const auto& light : lights)
	{
		const auto lightCenterPoints = light.getCenterPoints();
		const float xCenter = lightCenterPoints.first - 0.5f;
		const float yCenter = lightCenterPoints.second - 0.5f;
		_photonData.reserve(N_PHOTONS_TO_CAST*10u);
		_shadowPhotonData.reserve(N_PHOTONS_TO_CAST/10u);

		std::vector<std::thread> threads;
		size_t nThreads = std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency();

		for (size_t i = 0; i < N_PHOTONS_TO_CAST; i += nThreads)
		{			
			for (size_t i = 0; i < nThreads; i++)
			{
				threads.push_back(std::thread(
					&PhotonMap::photonMappingThreadFunction, this, std::ref(geometry)));
			}

			for (auto& thread : threads)
			{
				if(thread.joinable())
					thread.join();
			}
		}
		_photonMap.efficient_replace_and_optimise(_photonData);
		_shadowPhotonMap.efficient_replace_and_optimise(_shadowPhotonData);
		_photonData.clear();
		_shadowPhotonData.clear();
	}
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = endTime - startTime;
	std::cout << "done!\nPhoton map with " << _photonMap.size() << " photons constructed in " << duration.count() << " seconds.\n"
		<< static_cast<long long int>(_photonMap.size() - N_PHOTONS_TO_CAST) << " extra photons were created from monte carlo technique and specular surfaces.\n\n";
}

void PhotonMap::getPhotons(std::vector<PhotonNode>& foundPhotons, const PhotonNode& searchPoint)
{
	_photonMap.find_within_range(searchPoint, SEARCH_RANGE, std::back_inserter(foundPhotons));
}

bool PhotonMap::areShadowPhotonsPresent(const Vertex& intersectionPoint)
{
	PhotonNode p{ intersectionPoint };
	return _shadowPhotonMap.count_within_range(p, SEARCH_RANGE) > 0 ? true : false;
}

double PhotonMap::getPhotonFlux(const Vertex& intersectionPoint)
{
	PhotonNode searchPosition{ intersectionPoint };
	std::vector<PhotonNode> photons;
	photons.reserve(5u * N_PHOTONS_TO_CAST / 1000u);
	getPhotons(photons, searchPosition);

	double photonContrib{};
	for (const auto& p : photons)
		photonContrib += p.flux;
	photonContrib /= (PI * glm::pow(PhotonMap::SEARCH_RANGE, 2.0));

	return photonContrib;	
}

void PhotonMap::photonMappingThreadFunction(const SceneGeometry& geometry)
{
	const auto lightCenterPoints = geometry._ceilingLights[0].getCenterPoints();
	const float xCenter = lightCenterPoints.first - 0.5f;
	const float yCenter = lightCenterPoints.second - 0.5f;
	Photon initialPhoton = generateRandomPhotonFromLight(xCenter, yCenter);

	std::queue<Photon> photonQueue;
	photonQueue.push(std::move(initialPhoton));

	while (!photonQueue.empty())
	{
		std::vector<IntersectionSurface> pIntersects;
		Photon currentP = std::move(photonQueue.front());
		photonQueue.pop();

		photonIntersection(currentP, geometry, pIntersects);

		//One intersection, should only happen with diffuse surfaces
		if (pIntersects.size() == 1 && pIntersects[0].second == BRDF::DIFFUSE)
		{
			float pFlux = _deltaFlux * static_cast<float>(currentP.getColor().x);
			addPhoton(PhotonNode{ pIntersects[0].first._intersectPoint, pFlux, currentP.getNormalizedDirection() });
			handleMonteCarloPhoton(photonQueue, pIntersects[0].first, currentP);
		}
		else if (pIntersects.size() > 1) //Multiple intersections
		{
			if (pIntersects[0].second == BRDF::DIFFUSE)
			{
				float pFlux = _deltaFlux * static_cast<float>(currentP.getColor().x);
				addPhoton(PhotonNode{ pIntersects[0].first._intersectPoint, pFlux, currentP.getNormalizedDirection() });
				handleMonteCarloPhoton(photonQueue, pIntersects[0].first, currentP);
				addShadowPhotons(pIntersects);
			}
			else if (pIntersects[0].second == BRDF::REFLECTOR)
			{
				const IntersectionData tempInter = pIntersects[0].first;
				Photon reflectedPhoton = computeReflectedRay(tempInter._normal, currentP, tempInter._intersectPoint);
				photonQueue.push(std::move(reflectedPhoton));

				addShadowPhotons(pIntersects);
			}
			else if (pIntersects[0].second == BRDF::TRANSPARENT)
			{
				const IntersectionData tempInter = pIntersects[0].first;
				float incAngle = glm::angle(-currentP.getNormalizedDirection(), pIntersects[0].first._normal);
				double reflectionCoeff, n1, n2;
				bool rayIsTransmitted = shouldRayTransmit(n1, n2, reflectionCoeff, incAngle, currentP);

				if (rayIsTransmitted)
				{
					Photon refractedPhoton = computeRefractedRay(
						tempInter._normal, currentP, tempInter._intersectPoint, currentP.isInsideObject());
					refractedPhoton.setColor(refractedPhoton.getColor() * (1.f - reflectionCoeff));

					photonQueue.push(std::move(refractedPhoton));
				}
				Photon reflectedPhoton = computeReflectedRay(tempInter._normal, currentP, tempInter._intersectPoint);
				reflectedPhoton.setColor(reflectedPhoton.getColor() * reflectionCoeff);
				photonQueue.push(std::move(reflectedPhoton));
			}
		}
	}
}

void PhotonMap::addShadowPhotons(std::vector<IntersectionSurface>& inputData)
{
	std::lock_guard<std::mutex> tempLock{ this->_mutex };
	for (size_t i = 1; i < inputData.size(); i++)
	{
		auto tempInter = inputData[i].first;
		_shadowPhotonData.push_back(PhotonNode{ tempInter._intersectPoint });
	}
}

void PhotonMap::addPhoton(PhotonNode&& currentPhoton)
{
	std::lock_guard<std::mutex> tempLock{ this->_mutex };
	_photonData.push_back(std::move(currentPhoton));
}

Ray PhotonMap::generateRandomPhotonFromLight(const float x, const float y)
{
	const Vertex randPointOnLight{ x + _rng(_gen), y + _rng(_gen), 4.999f, 1.f };
	Direction randDir{ 0.f, 0.f, 1.f };
	randDir = glm::rotateY(randDir, randInclination(_gen, _rng));
	randDir = glm::rotateZ(randDir, randAzimuth(_gen, _rng));

	const Vertex randEndPoint = randPointOnLight - glm::vec4(randDir, 0.f);

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
			_gen,
			_rng);
		queue.push(std::move(generatedPhoton));
	}
}
