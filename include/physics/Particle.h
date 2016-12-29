#ifndef PHYS_PARTICLE_H_
#define PHYS_PARTICLE_H_
#include "Summation.h"
#include "AlloyMath.h"
namespace aly {
	// The information pertaining to a particle. The dynamic data is stored in a SumData.
	class Particle : public Summation
	{
	public:
		// Properties
		float mass;
		float3 x0;
		float perRegionMass;		// The amount of mass that goes to each region = mass * (1.0 / numParentRegions)
		// Dynamic values
		float3 x, v, f;
		float3 g;
		float3x3 R;				// The rotational component of the transformations of its parent regions -- useful in rendering
		// Relations
		std::vector<LatticeLocation *> parentRegions;
		// Collision stuff
		Particle *nextParticleInCollisionCell;	// Used in maintaining a linked list of particles in the collision cell
	};
	typedef std::shared_ptr<Particle> ParticlePtr;
}
#endif