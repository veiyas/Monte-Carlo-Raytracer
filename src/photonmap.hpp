#include <iostream>
#include <random>
#include <queue>
#include <chrono>
#include <kdtree.hpp>

#include "brdf.hpp"
#include "shapes.hpp"
#include "raycastingfunctions.hpp"



using Photon = Ray;

struct PhotonNode
{
	typedef float value_type;
	Vertex _pos;
	float flux;
	Direction _photonDir;

	value_type operator[](size_t n) const { return _pos[n]; }
};

class PhotonMap
{
public:
	PhotonMap() = default;
	PhotonMap(const SceneGeometry& geometry);

	//Fills the vector foundPhotons with the found photons within range
	//Returns true if it found shadowphotons
	void getPhotons(std::vector<PhotonNode>& foundPhotons, const PhotonNode& searchPoint);
	//Checks if shadow photons are present in range around searchPoint
	bool shadowPhotonsPresent(const PhotonNode& searchPoint);

private:
	KDTree::KDTree<3, PhotonNode> _photonMap;
	KDTree::KDTree<3, PhotonNode> _shadowPhotonMap;
	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;
	float _deltaFlux;

	void addShadowPhotons(std::vector<IntersectionSurface>& inputData);
	Ray generateRandomPhotonFromLight(const float x, const float y);
	float calculateDeltaFlux() const;
	void handleMonteCarloPhoton(std::queue<Ray>& queue, IntersectionData& inter, Photon& currentPhoton);

	static constexpr size_t N_PHOTONS_TO_CAST = 10000;
	static constexpr float SEARCH_RANGE = 1.f;
};