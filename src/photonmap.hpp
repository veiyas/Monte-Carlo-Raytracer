#include <iostream>
#include <random>
#include <kdtree.hpp>

#include "basic_types.hpp"
#include "shapes.hpp"
#include "ray.hpp"

#define TWO_PI 6.28318

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
	PhotonMap(std::vector<CeilingLight>& lights);
private:
	KDTree::KDTree<3, PhotonNode> _map;

	std::mt19937 _gen;
	std::uniform_real_distribution<float> _rng;

	static constexpr size_t N_PHOTONS_TO_CAST = 1000000;
};