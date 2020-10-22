#include <iostream>
#include <random>
#include <queue>
#include <chrono>
#include <kdtree.hpp>

#include "brdf.hpp"
#include "shapes.hpp"
#include "raycastingfunctions.hpp"

#define TWO_PI 6.28318

using Photon = Ray;

struct PhotonNode
{
	typedef float value_type;
	Vertex _pos;
	Direction _photonDir;
	bool isShadowPhoton;

	value_type operator[](size_t n) const { return _pos[n]; }
};

class PhotonMap
{
public:
	PhotonMap() = default;
	PhotonMap(const SceneGeometry& geometry);
private:
	KDTree::KDTree<3, PhotonNode> _map;
	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;

	void addShadowPhotons(
		std::vector<IntersectionSurface>& inputData,
		std::vector<PhotonNode>& photonMapData,
		Direction photonDir
	);
	Ray generateRandomPhotonFromLight(const float x, const float y);

	static constexpr size_t N_PHOTONS_TO_CAST = 10000;
};