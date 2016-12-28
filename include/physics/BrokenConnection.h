#ifndef PHYS_BROKENCONNECTION_H
#define PHYS_BROKENCONNECTION_H
#include "LatticeLocation.h"
namespace aly {
	// Used only in bookkeeping - remembering fractures
	struct BrokenConnection
	{
		LatticeLocation *a, *b;
		BrokenConnection(LatticeLocation *a, LatticeLocation *b) : a(a), b(b) {}
	};
}
#endif