#include "physics/stdafx.h"
#include <limits.h>
#include <functional>
#include <float.h>
#include <assert.h>
#include <stdlib.h>
#include "physics/FastLSM.h"
#include <queue>
#include "physics/jacobi.h"
#include <assert.h>
#include <stdlib.h>
namespace aly{
	void Body::CalculateInvariants()
	{
		printf("Calculating invariants...");

		// Calculate perRegionMass
		for each(Particle *particle in particles)
		{
			particle->perRegionMass = particle->mass / particle->parentRegions.size();
		}

		// Calculate region properties
		// Use fast summation
		for each(Particle *p in particles)
		{
			p->sumData.M(0,0) = p->perRegionMass;
			p->sumData.v = p->perRegionMass * p->x0;
		}
		SumParticlesToRegions();
		for each(Region *r in regions)
		{
			r->M = r->sumData.M(0, 0);
			r->Ex0 = r->sumData.v;
			r->c0 = r->Ex0 / r->M;
		}

		printf(" done.\n");
	}

	LatticeLocation* Body::GetLatticeLocation(int3 index)
	{
		std::map<int3, LatticeLocation*>::iterator found = lattice.find(index);
		if (found == lattice.end()) return NULL;
		else return found->second;
	}
}