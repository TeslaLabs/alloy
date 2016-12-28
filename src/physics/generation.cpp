#include "physics/stdafx.h"
#include "physics/FastLSM.h"
#include <queue>
#include <limits.h>
#include <functional>
#include <float.h>
#include <assert.h>
#include <algorithm>
namespace aly{

	Body::Body(float3 spacing)
	{
		this->spacing = spacing;
		alpha = 1.0f;
		fractureGoalWeight = 1.0f;
		fractureDistanceTolerance = 999.0f;
		fractureRotationTolerance = 999.0f;
		defaultParticleMass = 1.0f;
		w = 1;
		kRegionDamping = 0.5f;
		invariantsDirty = true;
	}

	void Body::AddParticle(int3 index)
	{
		// Initialize lattice location
		LatticeLocation *l = new LatticeLocation();
		latticeLocations.push_back(l);
		lattice[index] = l;
		l->index = index;
		l->body = this;
		l->region = NULL;

		// Set up the immediate neighborhood
		for (int xo = -1; xo <= 1; xo++)
		{
			for (int yo = -1; yo <= 1; yo++)
			{
				for (int zo = -1; zo <= 1; zo++)
				{
					int3 check = int3(index.x + xo, index.y + yo, index.z + zo);
					if (!(xo == 0 && yo == 0 && zo == 0) && GetLatticeLocation(check) != NULL)
					{
						l->immediateNeighbors.push_back(GetLatticeLocation(check));
						l->immediateNeighborsGrid[xo + 1][yo + 1][zo + 1] = GetLatticeLocation(check);
						GetLatticeLocation(check)->immediateNeighbors.push_back(l);
						GetLatticeLocation(check)->immediateNeighborsGrid[-xo + 1][-yo + 1][-zo + 1] = l;
					}
					else
					{
						l->immediateNeighborsGrid[xo + 1][yo + 1][zo + 1] = NULL;
					}
				}
			}
		}

		// Initialize particle
		l->particle = new Particle();
		particles.push_back(l->particle);
		l->particle->lp = l;
		l->particle->x0 = spacing * float3((float)index.x, (float)index.y, (float)index.z);
		l->particle->mass = defaultParticleMass;
		l->particle->x = l->particle->x0;
		l->particle->v = float3(0.0f);
		l->particle->f = float3(0.0f);
	}

	void Body::Finalize()
	{
		// Set the lattice points' immediateNeighbors, and decide based on this whether or not it is an edge
		for each(LatticeLocation *l in latticeLocations)
		{
			// Set whether it is an edge - i.e., doesn't have a full set of immediateNeighbors
			l->edge = (l->immediateNeighbors.size() != 26);

			// Build the neighborhood by breadth-first search
			l->CalculateNeighborhood();
		}

		// Generate the regions
		GenerateSMRegions();

		// Set the parent regions
		for each(Region *r in regions)
		{
			for each(Particle *p in r->particles)
			{
				p->parentRegions.push_back(r->lp);
			}
		}

		CalculateInvariants();

		InitializeCells();		// Cells help with rendering
	}

	void Body::GenerateSMRegions()
	{
		// Generate the regions from the lattice locations' neighborhoods
		printf("Generating regions and intermediate summations...");
		for each(LatticeLocation *l in latticeLocationsWithExistentRegions)
		{
			l->region = new Region();
			regions.push_back(l->region);
			l->region->lp = l;

			for each(LatticeLocation *l2 in l->neighborhood)
			{
				l->region->particles.push_back(l2->particle);
			}
			sort(l->region->particles.begin(), l->region->particles.end());

			// Initialize region
			l->region->eigenVectors = float3x3::identity();
		}

		// Generate the intermediate sub-summations (plates and bars)
		for each(Region *region in regions)
		{
			region->GenerateChildSums(1);
		}

		printf(" done.\n");
	}

	void Body::InitializeCells()
	{
		for each(LatticeLocation *l in latticeLocations)
		{
			l->cell = new Cell();
			cells.push_back(l->cell);
			l->cell->center = l;
		}

		// Have to call these after all cells have been created
		for each(Cell *cell in cells)
		{
			cell->Initialize();
		}
		for each(Cell *cell in cells)
		{
			cell->Initialize2();
		}
	}

}