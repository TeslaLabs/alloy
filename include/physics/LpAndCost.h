#ifndef PHYS_LPANDCOST_H
#define PHYS_LPANDCOST_H
#include "LatticeLocation.h"
namespace aly {
	// Just used in BFS for finding region extents
	struct LpAndCost
	{
		LatticeLocation *lp;
		float cost;

		LpAndCost(LatticeLocation *lp, float c) : lp(lp), cost(c) {}
		bool operator < (const LpAndCost &r) const
		{
			return cost < r.cost;
		}
		bool operator > (const LpAndCost &r) const
		{
			return cost > r.cost;
		}
	};
}
#endif
