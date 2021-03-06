#pragma once

#include <iostream>
#include <random>
#include <queue>
#include <chrono>
#include <kdtree.hpp>
#include <mutex>
#include <variant>
#include <algorithm>
#include <numeric>

#include "ray.hpp"
#include "brdf.hpp"
#include "shapes.hpp"
#include "raycastingfunctions.hpp"

using Photon = Ray; //For clarity

struct PhotonNode
{
	typedef float value_type;
	Vertex _pos;
	Radiance flux;
	Direction _photonDir;

	value_type operator[](size_t n) const { return _pos[n]; }
};

class PhotonMap
{
public:
	PhotonMap(const SceneGeometry& geometry);

	//Checks if shadow photons are present in range around searchPoint
	bool areShadowPhotonsPresent(const Vertex& intersectionPoint);
	//Calculates flux at intersectionPoint using all photins within range
	Radiance getPhotonRadianceContrib(const Direction& incomingDir,
		const SceneObject* const intersectObject, const IntersectionData& intersectionData);
private:
	KDTree::KDTree<3, PhotonNode> _photonMap;
	KDTree::KDTree<3, PhotonNode> _shadowPhotonMap;

	std::mutex _mutex;
	double _deltaFlux;

	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;

	void photonMapBuilderThreadFn(const SceneGeometry& geometry, std::vector<PhotonNode>& pMap,
		std::vector<PhotonNode>& spMap, size_t photonsToCast);

	void addShadowPhotons(std::vector<IntersectionSurface>& inputData, std::vector<PhotonNode>& spMap);
	void addPhoton(PhotonNode&& currentPhoton, std::vector<PhotonNode>& photonData);
	void getPhotons(std::vector<PhotonNode>& foundPhotons, const PhotonNode& searchPoint);
	Ray generateRandomPhotonFromLight(const float x, const float y);
	constexpr float calculateDeltaFlux() const;
	void handleMonteCarloPhoton(std::queue<Ray>& queue, IntersectionSurface& inter, Photon& currentPhoton);

	static constexpr float SEARCH_RANGE = 0.01f;
	static constexpr size_t N_PHOTONS_TO_CAST = 5'000'000;
};
