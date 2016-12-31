#ifndef PHYS_LATTICELOCATION_H
#define PHYS_LATTICELOCATION_H
#include "AlloyMath.h"
#include <vector>
namespace aly{
class Summation;
class Body;
class Chunk;
class Region;
class Particle;
class Cell;
class LatticeLocation
{
public:
	Body *body;
	Cell *cell;					// Primarily for rendering... we keep it in the physics engine to make things simple for users
	Particle *particle;
	Region *region;							// MAY BE nullptr
	int3 index;
	bool regionExists;			// Set to false if the region is identical to another region or otherwise turned off
	bool edge;					// Whether this is an edge - i.e., has less than the full number of immediateNeighbors


	unsigned int touch;
	float touchFloat;
								// Properties
							// The IMMEDIATE immediateNeighbors
	LatticeLocation():body(nullptr),cell(nullptr),particle(nullptr),region(nullptr),index(0),regionExists(false),edge(false),touch(0),touchFloat(0.0f) {

	}
	std::vector<LatticeLocation*> immediateNeighbors;
	LatticeLocation* immediateNeighborsGrid[3][3][3];	// Same as the pointers in the array, just indexed differently

														// Generated
	std::vector<LatticeLocation*> neighborhood;			// All particles up to w links away

														// The elements centered/living here

	std::vector<Summation*> sums[2];		// Sums that live here (not necc. centered). sums[0] = xSums, sums[1] = xySums

											// Used in some algorithms


	void CalculateNeighborhood();			// Will use a BFS to fill in neighborhood. Also sets regionExists
};
typedef std::shared_ptr<LatticeLocation> LatticeLocationPtr;
}
#endif