#include "physics/stdafx.h"
#include "physics/CellVertex.h"
#include "physics/FastLSM.h"
#include "physics/Cell.h"
#include <vector>
#include <algorithm>
namespace aly {
	CellVertex::CellVertex()
	{
		positionArbiter = NULL;

		for (unsigned int i = 0; i < 8; i++)
		{
			shareVertexCells[i] = NULL;
			partnerVertices[i] = NULL;
		}
	}

	void CellVertex::DeterminePositionArbiter()
	{
		Cell *positionArbiterCell;
		positionArbiter = NULL;
		// We point to the first of our partners that we are connected to for guidance
		std::vector<LatticeLocation*> &immediateNeighbors = owner->center->immediateNeighbors;
		for (unsigned int i = 0; i < 8; i++)
		{
			if (shareVertexCells[i] != NULL)
			{
				if (positionArbiter == NULL || shareVertexCells[i]->center->particle < positionArbiter)
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
	void CellVertex::HandleVertexSharerFracture()
	{
		DeterminePositionArbiter();
	}

	void CellVertex::UpdatePosition()
	{
		if (positionArbiter->parentRegions.size() <= 1)
			position = positionArbiter->g + materialPositionOffset;	// Position arbiter does not have a rotation defined
		else
			position = positionArbiter->g + positionArbiter->R * materialPositionOffset;
	}
}