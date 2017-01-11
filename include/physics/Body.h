#ifndef PHYS_BODY_H
#define PHYS_BODY_H
#include "Cell.h"
#include "LatticeLocation.h"
#include "LpAndCost.h"
#include "BrokenConnection.h"
#include "Particle.h"
#include <map>
#include <memory>
namespace aly {
	namespace softbody {
		class Body;
		class Particle;
		class Region;
		class Summation;
		class Body {
		public:
			// Properties
			std::string name;				// Used for universal reference
			float alpha;
			// Generation
			int w;
			float3 spacing;				// The spacing of the particles in the lattice
			float defaultParticleMass;
			std::map<int3, std::shared_ptr<LatticeLocation>> lattice;
			std::vector<std::shared_ptr<LatticeLocation>> latticeLocations;
			std::vector<LatticeLocation*> latticeLocationsWithExistentRegions;

			// Fracture
			bool fracturing;
			float fractureDistanceTolerance;
			float fractureGoalWeight;				// Just for distance - the blend between goal and actual positions for determining fracturing
			float fractureRotationTolerance;

			// Damping
			float kRegionDamping;

			// Elements
			std::list<std::shared_ptr<Particle>> particles;
			std::list<std::shared_ptr<Region>> regions;
			// Intermediate summations
			std::vector<std::shared_ptr<Summation>> sums[2];	// sums[0] = bars; sums[1] = plates

												// Misc.
			std::vector<std::shared_ptr<Cell>> cells;	// Useful for rendering - these are cubes centered at each particle with corners that deform appropriately
			bool invariantsDirty;		// Whether the invariants need to be recalculated -- simply set this to true after changing an invariant (e.g. particle mass) and the appropriate values will be recomputed automatically next time step

										// Generation
			Body(float3 spacing);
			void AddParticle(int3 index);
			void Finalize();			// After you add all the particles where you want them to be, you must finalize the object

										// Simulation
			void ShapeMatch();
			void CalculateParticleVelocities(float h);
			void PerformRegionDamping();
			void ApplyParticleVelocities(float h);
			void DoFracturing();
			void UpdateCellPositions();	// Actually useful only for rendering, so calling this is optional

										// Fast summation
			void SumParticlesToRegions();
			void SumRegionsToParticles();

			// Helper functions;
			void GenerateSMRegions();
			void CalculateInvariants();
			void InitializeCells();
			void RebuildRegions(std::vector<LatticeLocation*> &regen);		// Used in fracturing
			LatticeLocation* GetLatticeLocation(int3 index);
		};
		template <class T> void Remove(std::vector<T> &vec, const T t)
		{
			std::vector<T>::iterator new_end = remove(vec.begin(), vec.end(), t);
			vec.erase(new_end, vec.end());
		}

		typedef std::shared_ptr<Body> BodyPtr;
	}
}
#endif