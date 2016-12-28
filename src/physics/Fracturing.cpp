#include "physics/stdafx.h"
#include "physics/FastLSM.h"
#include <queue>
namespace aly {

	void Body::DoFracturing()
	{
		if (fracturing == false || (fractureDistanceTolerance >= 99 && fractureRotationTolerance >= 99))
			return;

		LatticeLocation *lp;

		// Detect fractures (immediateNeighbors that have strayed too far)
		std::vector<BrokenConnection> brokenConnections;
		bool fractureOccurred = false;
		for each(Particle *particle in particles)
		{
			lp = particle->lp;

			for each(LatticeLocation *neighbor in particle->lp->immediateNeighbors)
			{
				// Only do it in this case so we don't check the same links twice (just to save time)
				if (neighbor < lp)
				{
					bool broke = false;

					// Check DISTANCE tolerance
					if (fractureDistanceTolerance < 99)
					{
						float normalDist = length(neighbor->particle->x0 - particle->x0);
						float goalDist = length(neighbor->particle->g - particle->g);
						float posDist = length(neighbor->particle->x - particle->x);
						float actualDist = goalDist * fractureGoalWeight + (1.0f - fractureGoalWeight) * posDist;

						if (fabs(normalDist - actualDist) > fractureDistanceTolerance * normalDist)
						{
							broke = true;
						}
					}

					if (broke == false && fractureRotationTolerance < 99)
					{
						// Check ROTATION tolerance

						float3x3 rotationalDifference = particle->R - neighbor->particle->R;

						float sum = 0;
						for (int i = 0; i < 3; i++)
						{
							for (int j = 0; j < 3; j++)
							{
								sum += fabs(rotationalDifference(i,j));
							}
						}

						if (sum > fractureRotationTolerance)
						{
							broke = true;
						}
					}

					if (broke)
					{
						// FRACTURE!
						fractureOccurred = true;
						brokenConnections.push_back(BrokenConnection(lp, neighbor));
					}
				}
			}
		}

		if (fractureOccurred)
		{
			// Regenerate regions

			// Figure out the list of lattice locations that must be regenerated
			std::vector<LatticeLocation*> regen;
			for (unsigned int i = 0; i < brokenConnections.size(); i++)
			{
				LatticeLocation *a = brokenConnections[i].a;
				LatticeLocation *b = brokenConnections[i].b;

				if (find(a->immediateNeighbors.begin(), a->immediateNeighbors.end(), b) != a->immediateNeighbors.end())
				{
					// Erase the neighbor from my list of immediateNeighbors, and me from the neighbor's list
					a->immediateNeighbors.erase(find(a->immediateNeighbors.begin(), a->immediateNeighbors.end(), b));
					b->immediateNeighbors.erase(find(b->immediateNeighbors.begin(), b->immediateNeighbors.end(), a));
					// Also update the respective particle grids
					int3 offset = b->index - a->index;
					a->immediateNeighborsGrid[offset.x + 1][offset.y + 1][offset.z + 1] = NULL;
					b->immediateNeighborsGrid[-offset.x + 1][-offset.y + 1][-offset.z + 1] = NULL;
				}

				// They are edges now (they may have already been, too)
				a->edge = true;
				b->edge = true;

				for (unsigned int k = 0; k < a->particle->parentRegions.size(); k++)
				{
					LatticeLocation *toRegen = a->particle->parentRegions[k];
					if (find(regen.begin(), regen.end(), toRegen) == regen.end())
						regen.push_back(toRegen);
				}
			}

			// First update the LISTS of particles that would be in the affected regions (those to be regenerated = regen)
			std::vector<LatticeLocation*> changed;			// Those regions that were actually changed
			for each(LatticeLocation *lp in regen)
			{
				// What we are going to do is, for each location that was originally considered to be in this region,
				//  see if the direct path from that location to the center particle has any broken links.
				//  If there is no direct path, we remove it from the particle list.
				// This is faster than doing a true re-BFSing for the region, looks pretty much the same for non-pathological cases.
				bool particleRemoved = false;
				LatticeLocation *check;
				std::vector<LatticeLocation*>::iterator pI;
				for (pI = lp->neighborhood.begin(); pI != lp->neighborhood.end(); )
				{
					check = *pI;

					// Current is where we are now in tracing the line back, lp->index is the goal
					int3 current = check->index;
					bool brokeOut = false;
					while (current != lp->index)
					{
						int3 offset;
						int3 direction;

						offset = current - lp->index;

						// Gotta figure out the direction of the next step
						if (offset.x < 0) direction.x = 1;
						else if (offset.x > 0) direction.x = -1;
						else direction.x = 0;

						if (offset.y < 0) direction.y = 1;
						else if (offset.y > 0) direction.y = -1;
						else direction.y = 0;

						if (offset.z < 0) direction.z = 1;
						else if (offset.z > 0) direction.z = -1;
						else direction.z = 0;

						// Now check that it's a valid link
						LatticeLocation *currentLp = GetLatticeLocation(current);
						if (currentLp == NULL || currentLp->immediateNeighborsGrid[direction.x + 1][direction.y + 1][direction.z + 1] == NULL)
						{
							// We found a break in the chain
							pI = lp->neighborhood.erase(pI);
							brokeOut = true;
							particleRemoved = true;
							break;	// Break out of the while loop, as we can forget about this guy
									// DO NOT increment pI, because pI has been reset
						}
						else
						{
							// OK, it's valid, move along
							current += direction;
						}
					}
					if (!brokeOut)
					{
						pI++;		// Only need to set it if we broke out, as otherwise it was set by erase()
					}
				}

				if (particleRemoved)
					changed.push_back(lp);
			}

			// Now test if the regions that have been changed are too small (we don't handle regions with fewer than 3 particles)
			//  We could also check for redundancy (other identical regions), but we don't
			for each(LatticeLocation *lp in changed)
			{
				if (lp->neighborhood.size() <= 2)
				{
					lp->regionExists = false;
				}
			}

			printf("Rebulding regions: |changed| = %i\n", changed.size());

			RebuildRegions(changed);

			// Recalculate invariants, as they may have changed
			CalculateInvariants();

			for each(Cell *cell in cells)
			{
				cell->HandleVertexSharerFracture();
			}
		}
	}

	/*
	Fast fracturing:

	Once the lists have been updated:

	For each region
	For each plate I have
	if(this plate needs to be updated)
	plate[i]->numRef--;
	plate[i] = Find (plateParticles[i])
	if(plate[i] == NULL)
	plate[i] = Generate(plateParticles[i]);
	toAdd.push_back(plate[i]);

	for each plate
	if(plate[i]->numRef == 0)
	for each bar I have
	if(this bar needs to be updated)
	bar[i]->numRef--;
	bar[i] = Find (barParticles[i])
	if(bar[i] == NULL)
	bar[i] = Generate(barParticles[i]);


	for each bar
	if(bar[i]->numRef == 0)
	delete it
	*/

	template <class T>
	void Remove(std::vector<T> &vec, const T t)
	{
		std::vector<T>::iterator new_end = remove(vec.begin(), vec.end(), t);
		vec.erase(new_end, vec.end());
	}

	void Remove(std::vector<Summation*> &vec, const Summation *t)
	{
		std::vector<Summation*>::iterator new_end = remove(vec.begin(), vec.end(), t);
		vec.erase(new_end, vec.end());
	}

	void Body::RebuildRegions(std::vector<LatticeLocation*> &regen)
	{
		for each(LatticeLocation *l in regen)
		{
			Region *r = l->region;

			if (r != NULL)
			{
				// Copy the regions' particle lists from the lattice points to the regions
				r->particles.clear();
				for (unsigned int j = 0; j < r->lp->neighborhood.size(); j++)
					r->particles.push_back(r->lp->neighborhood[j]->particle);
				sort(r->particles.begin(), r->particles.end());

				std::vector<Summation*>::iterator plateI;
				// Check the case where r is now too small ( < 3 particles)
				if (r->particles.size() < 3)
				{
					for (plateI = r->children.begin(); plateI != r->children.end(); plateI++)
					{
						Summation *plate = *plateI;
						Remove(plate->parents, r);
					}
					r->children.clear();

					r->lp->regionExists = false;
					r->lp->region = NULL;
					Remove(regions, r);
					delete r;
				}
				else
				{
					// Generate the array of children, one per value of the sorting dimension
					Summation **childArray = new Summation*[r->maxDim - r->minDim + 1];
					int i;
					unsigned int m;
					for (i = 0; i < r->maxDim - r->minDim + 1; i++)
					{
						childArray[i] = new Summation();
					}

					// Sort the particles into their correct children
					for (m = 0; m < r->particles.size(); m++)
					{
						Particle *p = r->particles[m];

						childArray[p->lp->index[1] - r->minDim]->particles.push_back(p);
					}

					std::vector<Summation*> originalChildren;
					r->children.swap(originalChildren);

					for (plateI = originalChildren.begin(); plateI != originalChildren.end(); plateI++)
					{
						Summation *plate = *plateI;
						unsigned int newPlateIndex = plate->particles[0]->lp->index[1] - r->minDim;
						Summation *newPlate = childArray[newPlateIndex];
						sort(newPlate->particles.begin(), newPlate->particles.end());
						if (newPlate->particles.size() == plate->particles.size() && equal(plate->particles.begin(), plate->particles.end(), newPlate->particles.begin()))
						{
							// This plate is unchanged, so leave it as it was
							delete newPlate;
							childArray[newPlateIndex] = 0;
							r->children.push_back(plate);
						}
						else
						{
							// OK, we need to rebuild this plate
							Remove(plate->parents, r);

							if (newPlate->particles.size() == 0)
							{
								delete newPlate;
								childArray[newPlateIndex] = 0;
							}
							else
							{
								// Set up the plate
								newPlate->lp = newPlate->particles[0]->lp;

								// Check for an identical plate already existing
								Summation *identical = FindIdenticalSummation(newPlate->particles, 1);
								if (identical != NULL)
								{
									// There already exists such a plate
									delete newPlate;
									childArray[newPlateIndex] = 0;
									r->children.push_back(identical);
									identical->parents.push_back(r);
									sort(identical->parents.begin(), identical->parents.end());
								}
								else
								{
									// OK, we actually need to generate this plate

									// Record it
									r->children.push_back(newPlate);
									newPlate->parents.push_back(r);
									newPlate->lp->sums[1].push_back(newPlate);
									sums[1].push_back(newPlate);

									// Generate child sums
									std::vector<Summation*> newBars = newPlate->GenerateChildSums(0);
								}
							}
						}
					}
					sort(r->children.begin(), r->children.end());
				}
			}
		}

		//for each(Summation *plate in sums[1])
		std::vector<Summation*>::iterator iter;
		for (iter = sums[1].begin(); iter != sums[1].end(); )
		{
			if ((*iter)->parents.size() == 0)
			{
				Summation *plate = *iter;
				iter = sums[1].erase(iter);
				Remove(plate->lp->sums[1], plate);
				for each(Summation *bar in plate->children)
				{
					Remove(bar->parents, plate);
				}
				delete plate;
			}
			else
				iter++;
		}

		//for each(Summation *bar in sums[0])
		for (iter = sums[0].begin(); iter != sums[0].end(); )
		{
			if ((*iter)->parents.size() == 0)
			{
				Summation *bar = *iter;
				iter = sums[0].erase(iter);
				Remove(bar->lp->sums[0], bar);
				for each(Summation *particle in bar->children)
				{
					Remove(particle->parents, bar);
				}
				delete bar;
			}
			else
				iter++;
		}

		// Rebuild the particles' parentRegions lists
		for each(Particle *particle in particles)
		{
			particle->parentRegions.clear();
		}
		for each(Region *r in regions)
		{
			for each(Particle *p in r->particles)
			{
				p->parentRegions.push_back(r->lp);
			}
		}

		// Clear immediate neighbors for lattice locations that no longer have regions
		for each(LatticeLocation *l in latticeLocations)
		{
			if (l->particle->parentRegions.size() == 0 && l->immediateNeighbors.size() > 0)
			{
				for each(LatticeLocation *neighbor in l->immediateNeighbors)
				{
					Remove(neighbor->immediateNeighbors, l);
				}
				l->immediateNeighbors.clear();
				// We don't really need to update the immediate neighbors grid, as that is only used in generation
				//  (though we should update it if we want to re-BFS regions)
			}
		}
	}

}