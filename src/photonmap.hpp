#include <iostream>
#include <random>
#include <kdtree.hpp>

#include "basic_types.hpp"
#include "shapes.hpp"

struct Photon
{
	Vertex _pos;
	Direction _photonDir;
};

struct PhotonNode
{
	typedef float value_type;
	Photon _photon;
};

class PhotonMap
{
public:
	PhotonMap(std::vector<CeilingLight>& lights);

	static void initRNG();
private:
	KDTree::KDTree<3, PhotonNode> _map;

	static std::mt19937 _gen;
	static std::uniform_real_distribution<float> _rng;
};