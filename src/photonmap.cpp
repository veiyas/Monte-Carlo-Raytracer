#include "photonmap.hpp"

PhotonMap::PhotonMap(std::vector<CeilingLight>& lights)
	: _map{}
{
}

void PhotonMap::initRNG()
{
	_gen.seed(std::random_device{}());
	_rng = std::uniform_real_distribution<float>(0.f, 1.f);
}
