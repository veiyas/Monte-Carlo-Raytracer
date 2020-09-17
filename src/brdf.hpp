#pragma once

#include <iostream>
#include <optional>

class BRDF
{
public:
	BRDF(unsigned surfaceType);

	unsigned getSurfaceType() const { return _surfaceType; }
	enum
	{
		DIFFUSE,
		TRANSPARENT,
		REFLECTOR
	};
private:
	const unsigned _surfaceType;
};