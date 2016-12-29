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
	int3 index;
	Cell *cell;					// Primarily for rendering... we keep it in the physics engine to make things simple for users

								// Properties
	bool regionExists;			// Set to false if the region is identical to another region or otherwise turned off
	bool edge;					// Whether this is an edge - i.e., has less than the full number of immediateNeighbors

								// The IMMEDIATE immediateNeighbors
	std::vector<LatticeLocation*> immediateNeighbors;
	LatticeLocation* immediateNeighborsGrid[3][3][3];	// Same as the pointers in the array, just indexed differently

														// Generated
	std::vector<LatticeLocation*> neighborhood;			// All particles up to w links away

														// The elements centered/living here
	Particle *particle;
	Region *region;							// MAY BE NULL
	std::vector<Summation*> sums[2];		// Sums that live here (not necc. centered). sums[0] = xSums, sums[1] = xySums

											// Used in some algorithms
	unsigned int touch;
	float touchFloat;

	void CalculateNeighborhood();			// Will use a BFS to fill in neighborhood. Also sets regionExists
};
typedef std::shared_ptr<LatticeLocation> LatticeLocationPtr;
}
#endif