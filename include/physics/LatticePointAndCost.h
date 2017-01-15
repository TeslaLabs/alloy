#ifndef PHYS_LPANDCOST_H
#define PHYS_LPANDCOST_H
#include "LatticeLocation.h"
namespace aly {
	namespace softbody {

		// Just used in BFS for finding region extents
		struct LatticePointAndCost
		{
			LatticeLocation *lp;
			float cost;

			LatticePointAndCost(LatticeLocation *lp, float c) : lp(lp), cost(c) {}
			bool operator < (const LatticePointAndCost &r) const
			{
				return cost < r.cost;
			}
			bool operator > (const LatticePointAndCost &r) const
			{
				return cost > r.cost;
			}
		};
	}
}
#endif
