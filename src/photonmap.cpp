#include "photonmap.hpp"
#include "util.hpp"

PhotonMap::PhotonMap(const SceneGeometry& geometry)
	: _photonMap{}, _gen{ std::random_device{}() }, _rng{ 0.f, 1.f },
	_deltaFlux{ calculateDeltaFlux() }
{
	auto startTime = std::chrono::high_resolution_clock::now();

	size_t numCores = std::thread::hardware_concurrency();
	numCores = numCores == 0 ? 1 : numCores;

	std::cout << "Constructing photon map using " << numCores << " threads.\n";

	const size_t PHOTONS_PER_THREAD = N_PHOTONS_TO_CAST / numCores;
	std::vector<std::vector<PhotonNode>> pVectors, spVectors;
	pVectors.resize(numCores);
	spVectors.resize(numCores);

	std::cout << "Gathering photon data... ";
	auto startTime2 = std::chrono::high_resolution_clock::now();
	std::vector<std::thread> threads;
	for (size_t i{ 0 }; i < numCores; i++)
		threads.push_back(std::thread(
			&PhotonMap::photonMapBuilderThreadFn, this, std::ref(geometry), std::ref(pVectors[i]), std::ref(spVectors[i]), PHOTONS_PER_THREAD));
	for (auto& thread : threads)
		thread.join();

	//The most elaborate way to calculate the total amount of photons B)
	size_t nPhotonsCasted = std::accumulate(
		pVectors.begin(), pVectors.end(), 0u, [](const size_t& currSize, const std::vector<PhotonNode>& v){return currSize + v.size();});

	//Merge all photon data into one large vector
	std::vector<PhotonNode> allPhotons, allShadowPhotons;
	allPhotons.reserve(nPhotonsCasted);
	allShadowPhotons.reserve(nPhotonsCasted);
	for (size_t i = 0; i < pVectors.size(); i++)
	{
		std::move(pVectors[i].begin(), pVectors[i].end(), std::back_inserter(allPhotons));
		std::move(spVectors[i].begin(), spVectors[i].end(), std::back_inserter(allShadowPhotons));
	}
	auto endTime2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = endTime2 - startTime2;
	std::cout << "done!\nPhoton data gathered in " << durationFormat(duration) << ".\n";
	
	std::cout << "Creating photon map with gathered data... ";
	startTime2 = std::chrono::high_resolution_clock::now();
	_photonMap.efficient_replace_and_optimise(allPhotons);
	_shadowPhotonMap.efficient_replace_and_optimise(allShadowPhotons);
	endTime2 = std::chrono::high_resolution_clock::now();
	duration = endTime2 - startTime2;
	std::cout << "done!\nPhoton map constructed in " << durationFormat(duration) << ".\n";

	auto endTime = std::chrono::high_resolution_clock::now();
	duration = endTime - startTime;
	std::cout << "\nPhoton map with " << nPhotonsCasted
		<< " photons constructed in " << durationFormat(duration) << "\n"
		<< nPhotonsCasted - N_PHOTONS_TO_CAST << " photons created from diffuse and specular reflection.\n";
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

Radiance PhotonMap::getPhotonRadianceContrib(const Direction& incomingDir,
	const SceneObject* const intersectObject, const IntersectionData& intersectionData)
{
	PhotonNode searchPosition{ intersectionData._intersectPoint };
	std::vector<PhotonNode> photons;
	photons.reserve(5u * N_PHOTONS_TO_CAST / 1000u);
	getPhotons(photons, searchPosition);
	//std::cout << photons.size() << '.. \n';

	Radiance photonContrib{};
	for (const auto& p : photons)
	{
		double roughness = intersectObject->accessBRDF().computeBRDF(
			incomingDir,
			p._photonDir,
			glm::normalize(intersectionData._normal));
		roughness = glm::clamp(roughness, 0.0, 1.0);

		photonContrib += roughness * intersectObject->getColor() * p.flux;

		//if (someComponent(p.flux, [](double d) { return d <= 0; }))
		//{
		//	std::cout << p.flux << ' ' << "sawdust\n";
		//}
	}
	//if (someComponent(photonContrib, [](double d) { return d <= 0; }))
	//{
	//	std::cout << photonContrib << ' ' << "hej\n";
	//}

	//photonContrib /= (PI * glm::pow(PhotonMap::SEARCH_RANGE, 2.0));
	//if (photonContrib.x > 1.0 || photonContrib.y > 1.0 || photonContrib.z > 1.0)
	//	std::cout << photonContrib.x << " " << photonContrib.y << " " << photonContrib.z << ' ' << "\n";
	photonContrib /= (glm::pow(2 * PhotonMap::SEARCH_RANGE, 2.0)); // try to compensate for the non-euclidian distance

	return photonContrib;
}

void PhotonMap::photonMapBuilderThreadFn(const SceneGeometry& geometry, std::vector<PhotonNode>& pMap,
	std::vector<PhotonNode>& spMap, size_t photonsToCast)
{
	std::vector<CeilingLight> lights = geometry._ceilingLights;
	for (const auto& light : lights)
	{
		const auto lightCenterPoints = light.getCenterPoints();
		const float xCenter = lightCenterPoints.first - 0.5f;
		const float yCenter = lightCenterPoints.second - 0.5f;
		std::vector<PhotonNode>& photonData = pMap;
		photonData.reserve(photonsToCast*4u);

		for (size_t i = 0; i < photonsToCast; i++)
		{
			bool isEmittedByLight = true;
			std::queue<Photon> photonQueue;

			Photon initialPhoton = generateRandomPhotonFromLight(xCenter, yCenter);
			photonQueue.push(std::move(initialPhoton));

			while(!photonQueue.empty())
			{
				std::vector<IntersectionSurface> pIntersects;
				Photon currentP = std::move(photonQueue.front());
				photonQueue.pop();

				photonIntersection(currentP, geometry, pIntersects);

				if (pIntersects.size() != 0)
				{
					const unsigned pFirstIntersectSurfaceType = pIntersects[0].intersectionObject->getBRDF().getSurfaceType();
					if (pFirstIntersectSurfaceType == BRDF::DIFFUSE)
					{
						Radiance pFlux = _deltaFlux * currentP.getColor();
						addPhoton(PhotonNode{ pIntersects[0].intersectionData._intersectPoint, pFlux, currentP.getNormalizedDirection() },
							photonData);
						handleMonteCarloPhoton(photonQueue, pIntersects[0], currentP);

						if (isEmittedByLight)
							addShadowPhotons(pIntersects, spMap);
					}
					else if (pFirstIntersectSurfaceType == BRDF::REFLECTOR)
					{
						const IntersectionData tempInter = pIntersects[0].intersectionData;
						Photon reflectedPhoton = computeReflectedRay(tempInter._normal, currentP, tempInter._intersectPoint);
						reflectedPhoton.setColor(currentP.getColor()); //Radiance carries over
						photonQueue.push(std::move(reflectedPhoton));

						//std::cout << "reflection, i = " << i << ' ' << &currentP.getIntersectedObject()
						//	<< tempInter._t << '\n';
						if (isEmittedByLight)
							addShadowPhotons(pIntersects, spMap);
					}
					else if (pFirstIntersectSurfaceType == BRDF::TRANSPARENT)
					{
						const IntersectionData tempInter = pIntersects[0].intersectionData;
						float incAngle = glm::angle(-currentP.getNormalizedDirection(), pIntersects[0].intersectionData._normal);
						double reflectionCoeff, n1, n2;
						bool rayIsTransmitted = shouldRayTransmit(n1, n2, reflectionCoeff, incAngle, currentP);

					

						//std::cout << "refraction, i = " << i << ' ' //<< &currentP.getIntersectedObject() << ' '
						//	<< tempInter._t << pIntersects[0].first._intersectPoint << ' ' 
						//	<< currentP.isInsideObject() << ' ' << rayIsTransmitted << ' '
						//	<< currentP.getNormalizedDirection() << ' '
						//	<< '\n';

						//if (currentP.isInsideObject())
						//{
						//	std::cout << "hejkjl\n";
						//}

						//std::cout << reflectionCoeff << "hej\n";

						///* DEBUG */ rayIsTransmitted = true;
						if (rayIsTransmitted)
						{
							Photon refractedPhoton = computeRefractedRay(
								tempInter._normal, currentP, tempInter._intersectPoint, currentP.isInsideObject());
							//refractedPhoton.setColor(Color(1000000));
							refractedPhoton.setColor(currentP.getColor() * (1.f - reflectionCoeff));
							const auto& color = refractedPhoton.getColor();

							//if (color.x > 1.0 || color.y > 1.0 || color.z > 1.0)
							//	std::cout << color.x << " " << color.y << " " << color.z << "\n";

							//std::cout << currentP.getNormalizedDirection() << ' ' << refractedPhoton.getNormalizedDirection() << '\n';

							photonQueue.push(std::move(refractedPhoton));
						}
					
						// TODO This cutoff is somewhat arbitrary, and slightly different from in the rendering step.
						// This is bcs the information needed for that is not available here. It might be a good idea to fix this
						if (!currentP.isInsideObject())
						{
							Photon reflectedPhoton = computeReflectedRay(tempInter._normal, currentP, tempInter._intersectPoint);
							reflectedPhoton.setColor(reflectedPhoton.getColor() * reflectionCoeff);
							photonQueue.push(std::move(reflectedPhoton));
						}
					}
				}
				isEmittedByLight = false; // Remaining photons are reflected/refracted
			}
		}
	}
}

void PhotonMap::addShadowPhotons(std::vector<IntersectionSurface>& inputData, std::vector<PhotonNode>& spMap)
{
	//std::lock_guard<std::mutex> tempLock{ this->_mutex };
	for (size_t i = 1; i < inputData.size(); i++)
	{
		auto tempInter = inputData[i].intersectionData;
		spMap.push_back((PhotonNode{tempInter._intersectPoint}));
	}
}

void PhotonMap::addPhoton(PhotonNode&& currentPhoton, std::vector<PhotonNode>& photonData)
{
	std::lock_guard<std::mutex> tempLock{ this->_mutex };

	//if (someComponent(currentPhoton.flux, [](double d) { return d <= 0; }))
	//{
	//	std::cout << "hjlkmm\n";
	//}

	photonData.push_back(std::move(currentPhoton));
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

constexpr float PhotonMap::calculateDeltaFlux() const
{
	//The radiant flux is 1000 W, solid angle is pi (light emitted in hemisphere), area = 1
	//Therefore the radiance L0 is flux / (steradian * area) = 1000 / pi = 318.30989
	//From lecture 7 slide 10
	// 0.5pi * L0 = 500.000...

	//return 500.f / static_cast<float>(N_PHOTONS_TO_CAST);

	// Assuming 1 m2 light source
	constexpr float L0 = 1000.0f / glm::pi<float>(); // See lecture 10
	return glm::pi<float>() * L0 / static_cast<float>(N_PHOTONS_TO_CAST);
}

void PhotonMap::handleMonteCarloPhoton(std::queue<Ray>& queue, IntersectionSurface& inter, Photon& currentPhoton)
{
	float rand1 = _rng(_gen);
	float rand2 = _rng(_gen);

	if (rand1 + Config::monteCarloTerminationProbability() < 1.f)
	{
		Photon generatedPhoton = generateRandomReflectedRay(
			currentPhoton.getNormalizedDirection(),
			inter.intersectionData._normal,
			inter.intersectionData._intersectPoint,
			rand1,
			rand2);

		const double roughness = inter.intersectionObject->accessBRDF().computeBRDF(
			generatedPhoton.getNormalizedDirection(),
			-currentPhoton.getNormalizedDirection(),
			inter.intersectionData._normal);

		//TODO BRDF should be used here
		//Radiance is spread over a hemisphere, normalizing nominator is up for debate
		generatedPhoton.setColor(
			currentPhoton.getColor() *
			roughness *
			inter.intersectionObject->getColor() *
			(glm::pi<double>() / (1.0 - Config::monteCarloTerminationProbability())));
		queue.push(std::move(generatedPhoton));
	}
}
