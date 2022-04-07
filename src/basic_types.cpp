#include "basic_types.hpp"
#include "glm/gtx/string_cast.hpp"

Color safeDivide(const Color& num, const Color& den)
{
	double r = (den.r == 0) ? 0.0 : num.r / den.r;
	double g = (den.g == 0) ? 0.0 : num.g / den.g;
	double b = (den.b == 0) ? 0.0 : num.b / den.b;

	if (std::isnan(r) || std::isnan(g) || std::isnan(b) ||
		std::isinf(r) || std::isinf(g) || std::isinf(b))
		std::cout << glm::to_string(num) << '/' << glm::to_string(den) << "\n = (" << r << ", " << g << ", " << b << ") oh nooo\n";
	
	return Color(r, g, b);
}
