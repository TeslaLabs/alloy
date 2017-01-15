#include "physics/stdafx.h"
#include "physics/Cell.h"
#include "physics/Particle.h"
#include "physics/Body.h"
namespace aly {
	namespace softbody {

		int3 vertexOffset[8] = { int3(0,0,0), int3(0,0,1), int3(0,1,0), int3(0,1,1), int3(1,0,0), int3(1,0,1), int3(1,1,0), int3(1,1,1) };

		Cell::Cell()
		{
		}

		void Cell::initialize()
		{
			int3 p2 = center->index * 2;

			for (unsigned int i = 0; i < 8; i++)
			{
				CellVertex *v = &vertices[i];

				v->owner = this;
				float3 &spacing = center->body->spacing;
				v->materialPosition = center->particle->x0 + float3(spacing.x * ((float)vertexOffset[i].x - 0.5f), spacing.y * ((float)vertexOffset[i].y - 0.5f), spacing.z * ((float)vertexOffset[i].z - 0.5f));

				// Set up the vertex's shareVertexCells
				// The vertex's index
				int3 v2 = p2 + vertexOffset[i] * 2 - int3(1, 1, 1);
				v->v2 = v2;
				for (unsigned int j = 0; j < 8; j++)
				{
					// The shared point's offset
					int3 s2 = v2 + vertexOffset[j] * 2 - int3(1, 1, 1);
					int3 s = int3(s2.x / 2, s2.y / 2, s2.z / 2);

					if (center->body->getLatticeLocation(s) != NULL)
						v->shareVertexCells[j] = (Cell*)center->body->getLatticeLocation(s)->cell;
				}
			}

			// Set up the connected cells
			for(LatticeLocation *lp : center->immediateNeighbors){
				connectedCells.push_back(lp->cell);
			}
			connectedCells.push_back(this);		// I'm always connected to myself
		}

		void Cell::initialize2()
		{
			// For each vertex
			for (unsigned int i = 0; i < 8; i++)
			{
				CellVertex *v = &vertices[i];

				// Set up the partnerVertices
				// For each shareVertexCell
				for (unsigned int j = 0; j < 8; j++)
				{
					Cell *shared = v->shareVertexCells[j];
					if (shared != NULL)
					{
						// For each of the vertices of the shareVertexCell
						for (unsigned int l = 0; l < 8; l++)
						{
							if (shared->vertices[l].v2 == v->v2)//shared->vertices[l].materialPosition == v->materialPosition)
							{
								// That is our shared buddy
								v->partnerVertices[j] = &shared->vertices[l];
							}
						}
						assert(v->partnerVertices[j]);
					}
				}

				v->determinePositionArbiter();
				assert(v->positionArbiter);
			}
		}

		void Cell::handleVertexSharerFracture()
		{
			for (unsigned int i = 0; i < 8; i++)
			{
				vertices[i].handleVertexSharerFracture();
			}
		}

		void Cell::updateVertexPositions()
		{
			for (int i = 0; i < 8; i++)
			{
				vertices[i].updatePosition();
			}
		}
	}
}
