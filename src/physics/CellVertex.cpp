#include "physics/stdafx.h"
#include "physics/CellVertex.h"
#include "physics/FastLSM.h"
#include "physics/Cell.h"
#include <vector>
#include <algorithm>
namespace aly {
	namespace softbody {

		CellVertex::CellVertex()
		{
			positionArbiter = nullptr;

			for (unsigned int i = 0; i < 8; i++)
			{
				shareVertexCells[i] = nullptr;
				partnerVertices[i] = nullptr;
			}
		}

		void CellVertex::determinePositionArbiter()
		{
			Cell *positionArbiterCell;
			positionArbiter = nullptr;
			// We point to the first of our partners that we are connected to for guidance
			std::vector<LatticeLocation*> &immediateNeighbors = owner->center->immediateNeighbors;
			for (unsigned int i = 0; i < 8; i++)
			{
				if (shareVertexCells[i] != nullptr)
				{
					if (positionArbiter == nullptr || shareVertexCells[i]->center->particle < positionArbiter)
					{
						if (shareVertexCells[i] == owner || find(immediateNeighbors.begin(), immediateNeighbors.end(), shareVertexCells[i]->center) != immediateNeighbors.end())
						{
							positionArbiterVertex = partnerVertices[i];
							positionArbiterCell = shareVertexCells[i];
							positionArbiter = shareVertexCells[i]->center->particle;
							//positionArbiterParticleIndex = positionArbiter->particleIndex;
							materialPositionOffset = positionArbiterVertex->materialPosition - positionArbiter->x0;
							materialPosition = positionArbiterVertex->materialPosition;
							//return;
						}
					}
				}
			}
		}

		// Update in response to a fracture occuring in one of the cells that share this vertex
		void CellVertex::handleVertexSharerFracture()
		{
			determinePositionArbiter();
		}

		void CellVertex::updatePosition()
		{
			if (positionArbiter->parentRegions.size() <= 1)
				position = positionArbiter->g + materialPositionOffset;	// Position arbiter does not have a rotation defined
			else
				position = positionArbiter->g + positionArbiter->R * materialPositionOffset;
		}
	}
}