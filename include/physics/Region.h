#ifndef PHYS_REGION_H
#define PHYS_REGION_H
#include "Summation.h"
#include "AlloyMath.h"
namespace aly {
	namespace softbody {

		// Just as with Particle, it only holds the info - the dynamic properties are in a SumData
		class Region : public Summation {
		public:
			// Precomputed properties
			float3 Ex0;
			float3 c0;
			float M;
			// Dynamic properties
			float3 c;
			float3x3 A;
			// Tr is decomposed into the rotation and translation parts
			float3 t;
			float3x3 R;
		};
		typedef std::shared_ptr<Region> RegionPtr;
	}
}
#endif
