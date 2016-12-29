#ifndef PHYS_SUMMATION_H
#define PHYS_SUMMATION_H
#include "SumData.h"
namespace aly{
class LatticeLocation;
class Particle;

// Represents a region of the object that will be summed independently, usually as a
//  building block in generating the region sums. Defined principally by the list
//  of particles it contains. Will be responsible for generating its own children.
class Summation		// Particle, XSum, XYSum, Region
{
public:
	LatticeLocation *lp;	// For XSum and XYSum, may not actually be "centered" here,
							//  just lives here. This is useful in looking for same-content
							//  Sums during generation

	std::vector<Particle*> particles;
	std::vector<Summation*> children;
	std::vector<Summation*> parents;
	std::vector<Summation*>* connections[2];		// Pointers to above (children, parents)
	int minDim, maxDim;			// The range along the split dimension

	//__declspec(align(16)) 
	SumData sumData;

	Summation();
	void FindParticleRange(int dimension, int *minDim, int *maxDim);
	std::vector<std::shared_ptr<Summation>> GenerateChildSums(int childLevel);		// Returns the child summations that were generated

	void SumFromChildren();
	void SumFromParents();
	void PerformSummation(int direction);		// Generalization
};

Summation *FindIdenticalSummation(std::vector<Particle*> &particles, int myLevel);		// myLevel is 0 for XSums, 1 for XYSums
typedef std::shared_ptr<Summation> SummationPtr;
}
#endif