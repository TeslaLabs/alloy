#include "physics/stdafx.h"
#include "physics/FastLSM.h"
#include <queue>
#include <limits.h>
#include <functional>
#include <float.h>
#include <assert.h>
#include <string>
#include "physics/FastLSM.h"
namespace aly {
	void Body::ShapeMatch()
	{
		// Recalculate the various invaraints (mainly region properties dependent on the particles) if necessary
		if (invariantsDirty)
		{
			CalculateInvariants();
			invariantsDirty = false;
		}

		// Set each particle's sumData in preparation for calculating F(mixi) and F(mixi0T)
		for each(Particle *particle in particles)
		{
			particle->sumData.v = particle->perRegionMass * particle->x;
			particle->sumData.M = outerProd(particle->perRegionMass * particle->x, particle->x0);
		}

		// Calculate F(mixi) and F(mixi0T)
		SumParticlesToRegions();

		// Shape match
		for each(Region *r in regions)
		{
			float3 Fmixi = r->sumData.v;
			float3x3 Fmixi0T = r->sumData.M;

			r->c = (1 / r->M) * Fmixi;										// Eqn. 9
			r->A = Fmixi0T -outerProd(r->M * r->c, r->c0);		// Enq. 11

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
				if (eigenValues[j] <= 0)
				{
					eigenValues[j] = 0.05f;
				}
				eigenValues[j] = InvSqrt(eigenValues[j]);
				DPrime(j, j)= eigenValues[j];
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
		for each(Particle *particle in particles)
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
		ret(0,0) = (v.z * v.z + v.y * v.y);
		ret(0,1) = (-v.x*v.y);
		ret(0,2) = (-v.x*v.z);
		ret(1,1) = (v.z*v.z + v.x*v.x);
		ret(1,2) = (-v.z*v.y);
		ret(2,2) = (v.y*v.y + v.x*v.x);
		ret(1,0) = ret(0,1);
		ret(2,0) = ret(0,2);
		ret(2,1) = ret(1,2);
		return ret;
	}

	// Damp velocity per region according to the method of Mueller et al. 2006 (Position Based Dynamics), optimized for LSM as described in the paper
	void Body::PerformRegionDamping()
	{
		if (kRegionDamping == 0.0)
			return;

		// Set the data needed to calculate F(mivi), F(mix~ivi) and F(mix~ix~iT)
		for each(Particle *particle in particles)
		{
			// This is for F(mivi)
			particle->sumData.v = particle->perRegionMass * particle->v;

			// This is for F(mix~ivi)
			particle->sumData.M.x=cross(particle->x,particle->sumData.v);

			// This is for F(mix~ix~iT)
			// We take advantage of the fact that this is a symmetric matrix to squeeze the data in the standard SumData
			float3 x = particle->x;
			particle->sumData.M(2,1) = particle->perRegionMass * (x.z * x.z + x.y * x.y);
			particle->sumData.M(0,1) = particle->perRegionMass * (-x.x*x.y);
			particle->sumData.M(0,2) = particle->perRegionMass * (-x.x*x.z);
			particle->sumData.M(1,1) = particle->perRegionMass * (x.z*x.z + x.x*x.x);
			particle->sumData.M(1,2) = particle->perRegionMass * (-x.z*x.y);
			particle->sumData.M(2,2) = particle->perRegionMass * (x.y*x.y + x.x*x.x);
		}

		SumParticlesToRegions();

		for each(Region *region in regions)
		{
			float3 v = float3(0.0f);
			float3 L = float3(0.0f);
			float3x3 I = float3x3::zero();

			// Rebuild the original symmetric matrix from the reduced data
			float3x3 &M = region->sumData.M;
			float3x3 FmixixiT;
			M(0, 0) = M(2, 1);
			M(0, 1) = M(0, 1);
			M(0, 2) = M(0, 2);

			M(1, 0) = M(0, 1);
			M(1, 1) = M(1, 1);
			M(1, 2) = M(1, 2);

			M(2, 0) = M(0, 2);
			M(2, 1) = M(1, 2);
			M(2, 2) = M(2, 2);


			// Calculate v, L, I, w
			v = (1.0f / region->M) * region->sumData.v;							// Eqn. 14
			L = M.x - cross(region->c,region->sumData.v);
			I = FmixixiT - region->M * MrMatrix(region->c);

			float3 w;
			w = inverse(I) * L;

			// Set the data needed to apply this to the particles
			region->sumData.v = v;
			region->sumData.M.x=w;
			region->sumData.M.y= cross(w,region->c);
		}

		SumRegionsToParticles();

		// Apply calculated damping
		for each(Particle *particle in particles)
		{
			if (particle->parentRegions.size() > 0)
			{
				float3 Fv = particle->sumData.v;
				float3 Fw = particle->sumData.M.x;
				float3 Fwc = particle->sumData.M.y;
				float3 dv = (1.0f / particle->parentRegions.size()) * (Fv + cross(Fw,particle->x) - Fwc - (float)particle->parentRegions.size() * particle->v);

				// Bleed off non-rigid motion
				particle->v = particle->v + kRegionDamping * dv;
			}
		}
	}

	void Body::CalculateParticleVelocities(float h)
	{
		for each(Particle *particle in particles)
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
		for each(Particle *particle in particles)
		{
			particle->x = particle->x + h * particle->v;		// Eqn. 2
		}
	}

	void Body::UpdateCellPositions()
	{
		for each(Cell *cell in cells)
		{
			cell->UpdateVertexPositions();
		}
	}

	void Body::SumParticlesToRegions()
	{
		for each(Summation *bar in sums[0])
		{
			bar->SumFromChildren();
		}
		for each(Summation *plate in sums[1])
		{
			plate->SumFromChildren();
		}
		for each(Region *region in regions)
		{
			region->SumFromChildren();
		}
	}

	void Body::SumRegionsToParticles()
	{
		for each(Summation *plate in sums[1])
		{
			plate->SumFromParents();
		}
		for each(Summation *bar in sums[0])
		{
			bar->SumFromParents();
		}
		for each(Particle *particle in particles)
		{
			particle->SumFromParents();
		}
	}

}