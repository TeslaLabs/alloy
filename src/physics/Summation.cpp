#include "physics/stdafx.h"
#include "physics/Summation.h"
#include "physics/Body.h"
#include "physics/Particle.h"
namespace aly {
	namespace softbody {

		Summation::Summation() : lp(nullptr)
		{
			connections[0] = &children;
			connections[1] = &parents;
		}

		std::vector<SummationPtr> Summation::GenerateChildSums(int childLevel)
		{
			int i;
			unsigned int m;
			Particle *p;

			int splitDimension = childLevel;
			FindParticleRange(splitDimension, &minDim, &maxDim);

			std::vector<SummationPtr> newSums;

			// Generate the array of children, one per value of the sorting dimension
			std::vector<SummationPtr> childArray(maxDim - minDim + 1);
			for (i = 0; i < maxDim - minDim + 1; i++)
			{
				childArray[i].reset(new Summation());
			}

			// Sort the particles into their correct children
			for (m = 0; m < particles.size(); m++)
			{
				p = particles[m];

				childArray[p->lp->index[splitDimension] - minDim]->particles.push_back(p);
			}

			// Now process each child
			for (i = minDim; i <= maxDim; i++)
			{
				SummationPtr child = childArray[i - minDim];

				if (!child->particles.empty()) {
					sort(child->particles.begin(), child->particles.end());
					// Set the lp to the first particle's lp - it doesn't matter which particle, really, as long as it's
					//  the lp of one of the particles so it can be found when others are searching for identical sums
					child->lp = child->particles[0]->lp;

					// Now figure out if this sum is identical to any other
					Summation *identical = FindIdenticalSummation(child->particles, childLevel);

					if (identical != nullptr)
					{
						// Use the guy we found instead
						children.push_back(identical);
						identical->parents.push_back(this);
					}
					else
					{
						newSums.push_back(child);
						// Finally, register it with me and with the lattice point
						children.push_back(child.get());
						child->parents.push_back(this);
						child->lp->sums[childLevel].push_back(child.get());
						lp->body->sums[childLevel].push_back(child);

						// Have the child generate ITS child sums
						if (childLevel > 0)
							child->GenerateChildSums(childLevel - 1);
						else
						{
							// The child is a bar, so just connect it to its particles
							for (unsigned int m = 0; m < child->particles.size(); m++)
							{
								p = child->particles[m];
								child->children.push_back(p);
								p->parents.push_back(child.get());
							}
						}
					}
				}
			}

			return newSums;
		}

		// Find the extents of a particle bounding box in the given dimension
		void Summation::FindParticleRange(int dimension, int *minDim, int *maxDim)
		{
			unsigned int m;
			Particle *p;

			*minDim = std::numeric_limits<int>::max();
			*maxDim = std::numeric_limits<int>::min();

			for (m = 0; m < particles.size(); m++)
			{
				p = particles[m];

				if (p->lp->index[dimension] < *minDim)
					*minDim = p->lp->index[dimension];
				if (p->lp->index[dimension] > *maxDim)
					*maxDim = p->lp->index[dimension];
			}
		}

		// myLevel is 0 for XSums, 1 for XYSums
		Summation* FindIdenticalSummation(std::vector<Particle*> &particles, int myLevel)
		{
			unsigned int m, q;

			for (m = 0; m < particles.size(); m++)
			{
				LatticeLocation *checkLp = particles[m]->lp;

				for (q = 0; q < checkLp->sums[myLevel].size(); q++)
				{
					std::vector<Particle*> *checkParticles = &(checkLp->sums[myLevel][q]->particles);
					if (checkParticles->size() == particles.size() && equal(particles.begin(), particles.end(), checkParticles->begin()))
					{
						return checkLp->sums[myLevel][q];
					}
				}
			}

			return nullptr;
		}

		void Summation::SumFromChildren()
		{
			PerformSummation(0);
		}

		void Summation::SumFromParents()
		{
			PerformSummation(1);
		}

		// Slightly optimized using MMX operations (sum 4 floats in one op)	// Actually, no, it's not. Problems with aligning the memory in this version.
		void Summation::PerformSummation(int direction)
		{
			std::vector<Summation*> *sources = connections[direction];
			if (sources->size() == 0)
			{
				return;
			}
			else if (sources->size() == 1)
			{
				sumData.v = (*sources)[0]->sumData.v;
				sumData.M = (*sources)[0]->sumData.M;
				//sumData.m1 = (*sources)[0]->sumData.m1;
				//sumData.m2 = (*sources)[0]->sumData.m2;
				//sumData.m3 = (*sources)[0]->sumData.m3;
			}
			else
			{
				sumData.v = (*sources)[0]->sumData.v + (*sources)[1]->sumData.v;
				sumData.M = (*sources)[0]->sumData.M + (*sources)[1]->sumData.M;
				//sumData.m1 = _mm_add_ps((*sources)[0]->sumData.m1, (*sources)[1]->sumData.m1);
				//sumData.m2 = _mm_add_ps((*sources)[0]->sumData.m2, (*sources)[1]->sumData.m2);
				//sumData.m3 = _mm_add_ps((*sources)[0]->sumData.m3, (*sources)[1]->sumData.m3);
				for (unsigned int j = 2; j < sources->size(); j++)
				{
					sumData.v += (*sources)[j]->sumData.v;
					sumData.M += (*sources)[j]->sumData.M;
					//sumData.m1 = _mm_add_ps(sumData.m1, (*sources)[j]->sumData.m1);
					//sumData.m2 = _mm_add_ps(sumData.m2, (*sources)[j]->sumData.m2);
					//sumData.m3 = _mm_add_ps(sumData.m3, (*sources)[j]->sumData.m3);
				}
			}
		}
	}
}
