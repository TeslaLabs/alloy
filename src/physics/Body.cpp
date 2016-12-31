#include "physics/stdafx.h"
#include "physics/Body.h"
#include "physics/Region.h"
#include <limits.h>
#include <functional>
#include <float.h>
#include <assert.h>
#include <stdlib.h>
#include <queue>
#include <assert.h>
#include <stdlib.h>
namespace aly {

	Body::Body(float3 spacing):spacing(spacing)
	{
		alpha = 1.0f;
		fractureGoalWeight = 1.0f;
		fractureDistanceTolerance = 999.0f;
		fractureRotationTolerance = 999.0f;
		defaultParticleMass = 1.0f;
		w = 1;
		kRegionDamping = 0.5f;
		invariantsDirty = true;
	}

	void Body::AddParticle(int3 index){
		// Initialize lattice location
		LatticeLocationPtr l(new LatticeLocation());
		latticeLocations.push_back(l);
		lattice[index] = l;
		l->index = index;
		l->body = this;
		l->region = nullptr;
		// Set up the immediate neighborhood
		for (int xo = -1; xo <= 1; xo++){
			for (int yo = -1; yo <= 1; yo++){
				for (int zo = -1; zo <= 1; zo++){
					int3 check = int3(index.x + xo, index.y + yo, index.z + zo);
					LatticeLocation* latPt = GetLatticeLocation(check);
					if (!(xo == 0 && yo == 0 && zo == 0) && latPt != nullptr){
						l->immediateNeighbors.push_back(latPt);
						l->immediateNeighborsGrid[xo + 1][yo + 1][zo + 1] = latPt;
						latPt->immediateNeighbors.push_back(l.get());
						latPt->immediateNeighborsGrid[-xo + 1][-yo + 1][-zo + 1] = l.get();
					} else {
						l->immediateNeighborsGrid[xo + 1][yo + 1][zo + 1] = nullptr;
					}
				}
			}
		}
		// Initialize particle
		ParticlePtr particle(new Particle());
		l->particle = particle.get();
		particles.push_back(particle);
		l->particle->lp = l.get();
		float3 pos = spacing * float3((float)index.x, (float)index.y, (float)index.z);;
		l->particle->x0 = pos;
		l->particle->mass = defaultParticleMass;
		l->particle->x = pos;
		l->particle->v = float3(0.0f);
		l->particle->f = float3(0.0f);
		l->particle->R = float3x3::identity();
	}

	void Body::Finalize()
	{
		// Set the lattice points' immediateNeighbors, and decide based on this whether or not it is an edge
		for(LatticeLocationPtr l : latticeLocations)
		{
			// Set whether it is an edge - i.e., doesn't have a full set of immediateNeighbors
			l->edge = (l->immediateNeighbors.size() != 26);
			// Build the neighborhood by breadth-first search
			l->CalculateNeighborhood();
		}
		// Generate the regions
		GenerateSMRegions();
		// Set the parent regions
		for(RegionPtr r : regions){
			for(Particle* p : r->particles){
				p->parentRegions.push_back(r->lp);
			}
		}

		CalculateInvariants();
		InitializeCells();		// Cells help with rendering
		UpdateCellPositions();
	}

	void Body::GenerateSMRegions()
	{
		// Generate the regions from the lattice locations' neighborhoods
		printf("Generating regions and intermediate summations...");
		for(LatticeLocation* l : latticeLocationsWithExistentRegions)
		{
			RegionPtr region(new Region());
			l->region = region.get();
			regions.push_back(region);
			l->region->lp = l;
			for(LatticeLocation *l2 : l->neighborhood){
				l->region->particles.push_back(l2->particle);
			}
			sort(l->region->particles.begin(), l->region->particles.end());

			// Initialize region
			l->region->eigenVectors = float3x3::identity();
		}

		// Generate the intermediate sub-summations (plates and bars)
		for(RegionPtr region : regions)
		{
			region->GenerateChildSums(1);
		}

		printf(" done.\n");
	}

	void Body::InitializeCells()
	{
		for(LatticeLocationPtr l : latticeLocations)
		{
			CellPtr cell(new Cell());
			l->cell = cell.get();
			cells.push_back(cell);
			cell->center = l.get();
		}

		// Have to call these after all cells have been created
		for(CellPtr cell : cells)
		{
			cell->Initialize();
		}
		for(CellPtr cell : cells)
		{
			cell->Initialize2();
		}
	}
	void Body::CalculateInvariants()
	{
		printf("Calculating invariants...");

		// Calculate perRegionMass
		for(ParticlePtr particle : particles){
			particle->perRegionMass = particle->mass / particle->parentRegions.size();
			std::cout << "Particle Mass " << particle->perRegionMass << std::endl;
		}

		// Calculate region properties
		// Use fast summation
		for(ParticlePtr p : particles){
			p->sumData.M(0, 0) = p->perRegionMass;
			p->sumData.v = p->perRegionMass * p->x0;
		}
		SumParticlesToRegions();
		for(RegionPtr r : regions){
			r->M = r->sumData.M(0, 0);
			r->Ex0 = r->sumData.v;
			r->c0 = r->Ex0 / r->M;
		}
		printf(" done.\n");
	}

	LatticeLocation* Body::GetLatticeLocation(int3 index)
	{
		std::map<int3, LatticeLocationPtr>::iterator found = lattice.find(index);
		if (found == lattice.end()) return nullptr;
		else return found->second.get();
	}

	void Body::DoFracturing()
	{
		if (fracturing == false || (fractureDistanceTolerance >= 99 && fractureRotationTolerance >= 99))
			return;

		LatticeLocation *lp;

		// Detect fractures (immediateNeighbors that have strayed too far)
		std::vector<BrokenConnection> brokenConnections;
		bool fractureOccurred = false;
		for(ParticlePtr particle : particles){
			lp = particle->lp;
			for(LatticeLocation *neighbor : particle->lp->immediateNeighbors){
				// Only do it in this case so we don't check the same links twice (just to save time)
				if (neighbor < lp){
					bool broke = false;

					// Check DISTANCE tolerance
					if (fractureDistanceTolerance < 99){
						float normalDist = length(neighbor->particle->x0 - particle->x0);
						float goalDist = length(neighbor->particle->g - particle->g);
						float posDist = length(neighbor->particle->x - particle->x);
						float actualDist = goalDist * fractureGoalWeight + (1.0f - fractureGoalWeight) * posDist;

						if (fabs(normalDist - actualDist) > fractureDistanceTolerance * normalDist){
							broke = true;
						}
					}

					if (broke == false && fractureRotationTolerance < 99){
						// Check ROTATION tolerance

						float3x3 rotationalDifference = particle->R - neighbor->particle->R;

						float sum = 0;
						for (int i = 0; i < 3; i++){
							for (int j = 0; j < 3; j++){
								sum += fabs(rotationalDifference(i, j));
							}
						}

						if (sum > fractureRotationTolerance){
							broke = true;
						}
					}

					if (broke){
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
					a->immediateNeighborsGrid[offset.x + 1][offset.y + 1][offset.z + 1] = nullptr;
					b->immediateNeighborsGrid[-offset.x + 1][-offset.y + 1][-offset.z + 1] = nullptr;
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
			for(LatticeLocation *lp : regen)
			{
				// What we are going to do is, for location that was originally considered to be in this region,
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
						LatticeLocation* currentLp = GetLatticeLocation(current);
						if (currentLp == nullptr || currentLp->immediateNeighborsGrid[direction.x + 1][direction.y + 1][direction.z + 1] == nullptr)
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
			for(LatticeLocation *lp : changed)
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

			for(CellPtr cell : cells)
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
	if(plate[i] == nullptr)
	plate[i] = Generate(plateParticles[i]);
	toAdd.push_back(plate[i]);

	for plate
	if(plate[i]->numRef == 0)
	for bar I have
	if(this bar needs to be updated)
	bar[i]->numRef--;
	bar[i] = Find (barParticles[i])
	if(bar[i] == nullptr)
	bar[i] = Generate(barParticles[i]);


	for bar
	if(bar[i]->numRef == 0)
	delete it
	*/

	template <class T>
	void Remove(std::vector<T> &vec, const T t)
	{
		std::vector<T>::iterator new_end = remove(vec.begin(), vec.end(), t);
		vec.erase(new_end, vec.end());
	}
	template <class T>
	void Remove(std::vector<std::shared_ptr<T>> &vec, const T* t)
	{
		for (auto iter = vec.begin(); iter != vec.end(); iter++) {
			std::shared_ptr<T> val = *iter;
			if (val.get() == t) {
				vec.erase(iter);
				if (vec.size() > 0) {
					iter--;
				}
			}
		}
	}
	void Remove(std::vector<Summation*> &vec, const Summation *t)
	{
		std::vector<Summation*>::iterator new_end = remove(vec.begin(), vec.end(), t);
		vec.erase(new_end, vec.end());
	}

	void Body::RebuildRegions(std::vector<LatticeLocation*> &regen)
	{
		for(LatticeLocation *l : regen)
		{
			Region *r = l->region;

			if (r != nullptr)
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
					r->lp->region = nullptr;
					Remove(regions, r);
					delete r;
				}
				else
				{
					// Generate the array of children, one per value of the sorting dimension
					std::vector<SummationPtr> childArray(r->maxDim - r->minDim + 1);
					int i;
					unsigned int m;
					for (i = 0; i < r->maxDim - r->minDim + 1; i++)
					{
						childArray[i].reset(new Summation());
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
						SummationPtr newPlate = childArray[newPlateIndex];
						sort(newPlate->particles.begin(), newPlate->particles.end());
						if (newPlate->particles.size() == plate->particles.size() && equal(plate->particles.begin(), plate->particles.end(), newPlate->particles.begin()))
						{
							// This plate is unchanged, so leave it as it was
							childArray[newPlateIndex] = 0;
							r->children.push_back(plate);
						}
						else
						{
							// OK, we need to rebuild this plate
							Remove(plate->parents, r);

							if (newPlate->particles.size() == 0)
							{
								childArray[newPlateIndex] = 0;
							}
							else
							{
								// Set up the plate
								newPlate->lp = newPlate->particles[0]->lp;

								// Check for an identical plate already existing
								Summation *identical = FindIdenticalSummation(newPlate->particles, 1);
								if (identical != nullptr)
								{
									// There already exists such a plate
									childArray[newPlateIndex] = 0;
									r->children.push_back(identical);
									identical->parents.push_back(r);
									sort(identical->parents.begin(), identical->parents.end());
								}
								else
								{
									// OK, we actually need to generate this plate

									// Record it
									r->children.push_back(newPlate.get());
									newPlate->parents.push_back(r);
									newPlate->lp->sums[1].push_back(newPlate.get());
									sums[1].push_back(newPlate);

									// Generate child sums
									std::vector<SummationPtr> newBars = newPlate->GenerateChildSums(0);
								}
							}
						}
					}
					sort(r->children.begin(), r->children.end());
				}
			}
		}

		//for(Summation *plate in sums[1])
		std::vector<SummationPtr>::iterator iter;
		for (iter = sums[1].begin(); iter != sums[1].end(); )
		{
			if ((*iter)->parents.size() == 0)
			{
				SummationPtr plate = *iter;
				iter = sums[1].erase(iter);
				Remove(plate->lp->sums[1], plate.get());
				for(Summation *bar : plate->children){
					Remove(bar->parents, plate.get());
				}
			}
			else
				iter++;
		}

		//for(Summation *bar in sums[0])
		for (iter = sums[0].begin(); iter != sums[0].end(); )
		{
			if ((*iter)->parents.size() == 0)
			{
				SummationPtr bar = *iter;
				iter = sums[0].erase(iter);
				Remove(bar->lp->sums[0], bar.get());
				for(Summation *particle : bar->children)
				{
					Remove(particle->parents, bar.get());
				}
			}
			else
				iter++;
		}

		// Rebuild the particles' parentRegions lists
		for(ParticlePtr particle : particles)
		{
			particle->parentRegions.clear();
		}
		for(RegionPtr r : regions)
		{
			for(Particle* p : r->particles)
			{
				p->parentRegions.push_back(r->lp);
			}
		}

		// Clear immediate neighbors for lattice locations that no longer have regions
		for(LatticeLocationPtr l : latticeLocations)
		{
			if (l->particle->parentRegions.size() == 0 && l->immediateNeighbors.size() > 0)
			{
				for(LatticeLocation* neighbor : l->immediateNeighbors)
				{
					Remove(neighbor->immediateNeighbors, l.get());
				}
				l->immediateNeighbors.clear();
				// We don't really need to update the immediate neighbors grid, as that is only used in generation
				//  (though we should update it if we want to re-BFS regions)
			}
		}
	}
	void Body::ShapeMatch()
	{
		// Recalculate the various invaraints (mainly region properties dependent on the particles) if necessary
		if (invariantsDirty)
		{
			CalculateInvariants();
			invariantsDirty = false;
		}

		// Set each particle's sumData in preparation for calculating F(mixi) and F(mixi0T)
		for(ParticlePtr particle : particles)
		{
			particle->sumData.v = particle->perRegionMass * particle->x;
			particle->sumData.M = outerProd(particle->perRegionMass * particle->x, particle->x0);
		}

		// Calculate F(mixi) and F(mixi0T)
		SumParticlesToRegions();

		// Shape match
		for(RegionPtr r : regions)
		{
			float3 Fmixi = r->sumData.v;
			float3x3 Fmixi0T = r->sumData.M;

			r->c = (1 / r->M) * Fmixi;										// Eqn. 9
			r->A = Fmixi0T - outerProd(r->M * r->c, r->c0);		// Enq. 11

																// ### Polar decompose Ar following the method in Mueller et al. 2005:

																// Multiply by the stored eigenvectors (for faster polar decomposition - see Extensions section of FastLSM paper)
			float3x3 S = transpose(r->A) * r->A;
			S = transpose(r->eigenVectors) * S * r->eigenVectors;
			float3x3 D;
			Eigen(S, r->eigenVectors, D);
			float eigenValues[3];
			float3x3 DPrime = float3x3::zero();
			for (int j = 0; j < 3; j++)
			{
				eigenValues[j] = D(j, j);
				if (eigenValues[j] <= 0){
					eigenValues[j] = 0.05f;
				}
				eigenValues[j] = InvSqrt(eigenValues[j]);
				DPrime(j, j) = eigenValues[j];
			}
			S = r->eigenVectors * DPrime * transpose(r->eigenVectors);
			// Now we can get the rotation part
			r->R = r->A * S;

			// ### Done with the polar decomposition

			// Test for inversion (flipping of the rest configuration)
			// Disabled for fracturing objects as it can cause some screwups with degenerate (planar, linear) regions
			if (determinant(r->R) < 0 && fracturing == false)
			{
				r->R *= -1.0f;
			}

			// Set the translation part
			r->t = r->c - r->R * r->c0;

			// Set the region's SumData in preparation for calculating F(Tr)
			r->sumData.M = r->R;
			r->sumData.v = r->t;
		}

		// Calculate F(Tr)
		SumRegionsToParticles();

		// Calculate goal positions for the particles
		for(ParticlePtr particle : particles)
		{
			float invNumParentRegions = 1.0f / particle->parentRegions.size();

			// Eqn. 12, split into rotation and translation
			particle->g = (invNumParentRegions * particle->sumData.M) * particle->x0 + invNumParentRegions * particle->sumData.v;

			// Store just the rotational part too; it's useful for rendering
			particle->R = invNumParentRegions * particle->sumData.M;
		}
	}

	float3x3 MrMatrix(float3 v)
	{
		float3x3 ret;
		ret(0, 0) = (v.z * v.z + v.y * v.y);
		ret(0, 1) = (-v.x*v.y);
		ret(0, 2) = (-v.x*v.z);
		ret(1, 1) = (v.z*v.z + v.x*v.x);
		ret(1, 2) = (-v.z*v.y);
		ret(2, 2) = (v.y*v.y + v.x*v.x);
		ret(1, 0) = ret(0, 1);
		ret(2, 0) = ret(0, 2);
		ret(2, 1) = ret(1, 2);
		return ret;
	}

	// Damp velocity per region according to the method of Mueller et al. 2006 (Position Based Dynamics), optimized for LSM as described in the paper
	void Body::PerformRegionDamping()
	{
		if (kRegionDamping == 0.0)
			return;

		// Set the data needed to calculate F(mivi), F(mix~ivi) and F(mix~ix~iT)
		for(ParticlePtr particle : particles)
		{
			// This is for F(mivi)
			particle->sumData.v = particle->perRegionMass * particle->v;

			// This is for F(mix~ivi)
			particle->sumData.M.x = cross(particle->x, particle->sumData.v);

			// This is for F(mix~ix~iT)
			// We take advantage of the fact that this is a symmetric matrix to squeeze the data in the standard SumData
			float3 x = particle->x;
			particle->sumData.M(2, 1) = particle->perRegionMass * (x.z * x.z + x.y * x.y);
			particle->sumData.M(0, 1) = particle->perRegionMass * (-x.x*x.y);
			particle->sumData.M(0, 2) = particle->perRegionMass * (-x.x*x.z);
			particle->sumData.M(1, 1) = particle->perRegionMass * (x.z*x.z + x.x*x.x);
			particle->sumData.M(1, 2) = particle->perRegionMass * (-x.z*x.y);
			particle->sumData.M(2, 2) = particle->perRegionMass * (x.y*x.y + x.x*x.x);
		}

		SumParticlesToRegions();

		for(RegionPtr region : regions)
		{
			float3 v = float3(0.0f);
			float3 L = float3(0.0f);
			float3x3 I;

			// Rebuild the original symmetric matrix from the reduced data
			const float3x3 &M = region->sumData.M;
			float3x3 FmixixiT;
			FmixixiT(0, 0) = M(2, 1);
			FmixixiT(0, 1) = M(0, 1);
			FmixixiT(0, 2) = M(0, 2);
			FmixixiT(1, 0) = M(0, 1);
			FmixixiT(1, 1) = M(1, 1);
			FmixixiT(1, 2) = M(1, 2);
			FmixixiT(2, 0) = M(0, 2);
			FmixixiT(2, 1) = M(1, 2);
			FmixixiT(2, 2) = M(2, 2);



			// Calculate v, L, I, w
			v = (1.0f / region->M) * region->sumData.v;							// Eqn. 14
			L = M.x - cross(region->c, region->sumData.v);
			I = FmixixiT - region->M * MrMatrix(region->c);
			std::cout << "I=" << I<< std::endl;
			float3 w = inverse(I) * L;

			// Set the data needed to apply this to the particles
			region->sumData.v = v;
			region->sumData.M.x = w;
			region->sumData.M.y = cross(w, region->c);
		}

		SumRegionsToParticles();

		// Apply calculated damping
		for(ParticlePtr particle : particles)
		{
			if (particle->parentRegions.size() > 0)
			{
				float3 Fv = particle->sumData.v;
				float3 Fw = particle->sumData.M.x;
				float3 Fwc = particle->sumData.M.y;
				float3 dv = (1.0f / particle->parentRegions.size()) * (Fv + cross(Fw, particle->x) - Fwc - (float)particle->parentRegions.size() * particle->v);

				// Bleed off non-rigid motion
				particle->v = particle->v + kRegionDamping * dv;
			}
		}
	}

	void Body::CalculateParticleVelocities(float h)
	{
		for(ParticlePtr particle : particles)
		{
			if (particle->parentRegions.size() == 0)
			{
				// We are just a lone particle flying about - no regions, so no goal position - so only account for fExt
				particle->v = particle->v + h * (particle->f / particle->mass);

				// Set the goal in case we want to render particle goals
				particle->g = particle->x;
			}
			else
			{
				// We have a goal position
				particle->v = particle->v + alpha * (particle->g - particle->x) / h + h * (particle->f / particle->mass);	// Eqn. 1
			}
			particle->f = float3(0.0f);			// Zero the force accumulator
		}
	}

	void Body::ApplyParticleVelocities(float h)
	{
		for(ParticlePtr particle : particles)
		{
			particle->x = particle->x + h * particle->v;		// Eqn. 2
		}
	}

	void Body::UpdateCellPositions()
	{
		for(CellPtr cell : cells)
		{
			cell->UpdateVertexPositions();
		}
	}

	void Body::SumParticlesToRegions()
	{
		for(SummationPtr bar : sums[0])
		{
			bar->SumFromChildren();
		}
		for(SummationPtr plate : sums[1])
		{
			plate->SumFromChildren();
		}
		for(RegionPtr region : regions)
		{
			region->SumFromChildren();
		}
	}

	void Body::SumRegionsToParticles()
	{
		for(SummationPtr plate : sums[1])
		{
			plate->SumFromParents();
		}
		for(SummationPtr bar : sums[0])
		{
			bar->SumFromParents();
		}
		for(ParticlePtr particle : particles)
		{
			particle->SumFromParents();
		}
	}
}