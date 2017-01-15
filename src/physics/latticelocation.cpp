#include "physics/stdafx.h"
#include "physics/LatticeLocation.h"
#include "physics/Body.h"
#include "physics/Region.h"
#include "physics/Particle.h"
#include <queue>
namespace aly {
	namespace softbody {

		void LatticeLocation::CalculateNeighborhood()
		{
			// Generate the set of lattice locations up to w steps from each lattice location

			neighborhood.clear();

			if (body->w == 1)
			{
				// We can just use the neighbor list
				neighborhood = immediateNeighbors;
				sort(neighborhood.begin(), neighborhood.end());
			}
			else
			{
				// Find the set of particles up to w steps away

				unsigned int newTouch = rand() % std::numeric_limits<int>::max();
				std::queue<LatticeLocation*> next;
				int currentDepth = 0;
				int remainingAtThisDepth = 1;
				int elementsAtNextDepth = 0;

				next.push(this);

				// Rather than doing an expensive search to see if we've already added a given lattice location, we just mark it when we add it
				touch = newTouch;

				while (!next.empty())
				{
					// Get "u" from the queue, add it to neighbors
					LatticeLocation* u = next.front();
					next.pop();
					neighborhood.push_back(u);

					if (currentDepth < body->w)
					{
						std::vector<LatticeLocation*>::const_iterator neighborI;
						// Add all u's neighbors to next
						for (neighborI = u->immediateNeighbors.begin(); neighborI != u->immediateNeighbors.end(); neighborI++)
						{
							LatticeLocation *neighbor = *neighborI;
							if (neighbor->touch != newTouch)
							{
								neighbor->touch = newTouch;
								next.push(neighbor);
								elementsAtNextDepth++;
							}
						}
					}

					remainingAtThisDepth--;
					if (remainingAtThisDepth == 0)
					{
						currentDepth++;
						remainingAtThisDepth = elementsAtNextDepth;
						elementsAtNextDepth = 0;
					}
				}

				sort(neighborhood.begin(), neighborhood.end());
			}

			// Set whether the region should be generated or not
			if (body->fracturing)
			{
				// We always generate the region if there is fracturing, because one region that starts out as identical to another may
				//  become different as a result of fracturing, and it's faster to just generate them all at the start rather than doing
				//  expensive tests for identicalness every time there is a fracture
				regionExists = true;
			}
			else
			{
				regionExists = true;
				// Check if we are a duplicate
				for (LatticeLocation* check : body->latticeLocationsWithExistentRegions)
				{
					if (check->neighborhood.size() == neighborhood.size() && equal(neighborhood.begin(), neighborhood.end(), check->neighborhood.begin()))
					{
						// We ARE a duplicate
						regionExists = false;
						break;
					}
				}
			}

			if (regionExists)
			{
				body->latticeLocationsWithExistentRegions.push_back(this);
			}
		}
	}
}
