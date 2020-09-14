#include "basic_types.hpp"

//Can be inherited to get the useful data defined
//Used for shadowrays as of
class PointLight
{
public:
	PointLight(Vertex position, Color _color);

	Vertex getPosition() const { return _position; }
protected: //Why is this bad again?
	Vertex _position;
	Color _color; //Perhaps not needed
};