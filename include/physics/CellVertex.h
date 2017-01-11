#ifndef PHYS_CELLVERTEX_H
#define PHYS_CELLVERTEX_H
#include "AlloyMath.h"
namespace aly {
	namespace softbody {

		class Particle;
		class Cell;
		class Body;

		// A class representing a vertex at a corner of a Cell
		class CellVertex
		{
		public:
			float3 position;					// The current (deformed) position
			float3 materialPosition;			// The original (undeformed) position

												// Used in determining the deformed position...
			Cell *owner;
			Particle *positionArbiter;			// There is just one arbiter of the vertex's position - maybe my cell's owner, mabye not.
			CellVertex *positionArbiterVertex;
			float3 materialPositionOffset;	// From the position arbiter
			Cell *shareVertexCells[8];			// The cells that share this vertex/corner
			CellVertex *partnerVertices[8];		// The other vertices with the same material position from other cells
			int3 v2;

			CellVertex();
			void DeterminePositionArbiter();
			void HandleVertexSharerFracture();	// Update in response to a fracture occuring in one of the cells that share this vertex

			void UpdatePosition();
		};
	}
}
#endif