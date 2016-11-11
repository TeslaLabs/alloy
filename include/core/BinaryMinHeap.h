/*
* Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#ifndef BINARYMINHEAP_H_
#define BINARYMINHEAP_H_
#include "AlloyMath.h"
#include "AlloyVolume.h"
#include "AlloyLocator.h"
namespace aly {
	template <class T,int C> struct Indexable {
		vec<int, C> index;
		T value;
		Indexable(const T& val=T(0)):index(0),value(val) {

		}
		Indexable(const vec<int, C>& index,const T& val) :index(index), value(val) {

		}
	};
template<class T, int C> class BinaryMinHeap {
protected:
	vec<int, C> dimensions;
	std::vector<Indexable<T, C> *> heapArray;
	std::vector<size_t> backPointers;
	size_t currentSize;
protected:
	inline size_t& getPointer(const vec<int, C>& idx) {
		switch (C) {
			case 1: return backPointers[idx[0]];break;
			case 2: return backPointers[idx[0]+ idx[1]*dimensions[0]];break;
			case 3: return backPointers[idx[0] + dimensions[0] * ( idx[1] + dimensions[1] *idx[2])];break;
			default: throw std::runtime_error(MakeString()<<"Heap dimension not supported [" << C << "]");
		}
	}
public:
	BinaryMinHeap(const vec<int,C>& dims):dimensions(dims),currentSize(0) {
		size_t sz = 1;
		for (int c = 0;c < C;c++) {
			sz *= dims[c];
		}
		backPointers.resize(sz,0);
	}
	void reserve(size_t capacity){
		heapArray.resize(capacity + 2,nullptr);
	}
	bool isEmpty() {
		return currentSize == 0;
	}
	Indexable<T,C>* peek() {
		if (isEmpty()) {
			throw std::runtime_error("Empty binary heap");
		}
		return heapArray[1];
	}
	size_t size() {
		return currentSize;
	}
	void change(const vec<int, C>& pos, Indexable<T,C>* x) {
		size_t index = getPointer(pos);
		Indexable<T,C>* v = heapArray[index];
		if (x != v) {
			heapArray[index] = x;
			if (x->value<v->value) {
				percolateUp(index);
			} else {
				percolateDown(index);
			}
		}
	}
	void change(const vec<int,C>& pos,T value) {
		size_t index = getPointer(pos);
		Indexable<T,C>* v = heapArray[index];
		if (value<v->value) {
			v->value=value;
			percolateUp(index);
		} else {
			v->value=value;
			percolateDown(index);
		}
	}
	void change(Indexable<T,C> node,T value) {
		change(node.index, value);
	}
	void add(Indexable<T,C>* x) {
		if (currentSize + 1 >= heapArray.size()) {
			resize();
		}
		size_t hole = ++currentSize;
		heapArray[0] = x;
		for (; x->value<heapArray[hole / 2]->value; hole /= 2) {
			heapArray[hole] = heapArray[hole / 2];
			getPointer(heapArray[hole]->index)=hole;
		}
		getPointer(x->index) = hole;
		heapArray[hole] = x;
	}
	Indexable<T,C>* remove() {
		Indexable<T,C>* minItem = peek();
		heapArray[1] = heapArray[currentSize--];
		percolateDown(1);
		return minItem;
	}
	void remove(Indexable<T,C>* item) {
		size_t idx=getPointer(item->index);
		heapArray[idx] = heapArray[currentSize--];
		percolateDown(idx);
	}
	void clear() {
		currentSize = 0;
		heapArray.clear();
		heapArray.shrink_to_fit();
		backPointers.assign(backPointers.size(), 0);
	}
protected:

	void buildHeap() {
		for (int i = currentSize / 2; i > 0; i--) {
			percolateDown(i);
		}
	}
	void percolateDown(size_t parent) {
		size_t child;
		Indexable<T,C>* tmp = heapArray[parent];
		if (tmp == nullptr) {
			return;
		}
		for (; parent * 2 <= currentSize; parent = child) {
			child = parent * 2;
			if (heapArray[child] == nullptr) {
				parent = child;
				break;
			}
			if (heapArray[child + 1] == nullptr) {
				parent = child + 1;
				break;
			}
			if (child != currentSize
					&& heapArray[child + 1]->value<heapArray[child]->value) {
				child++;
			}
			if (heapArray[child]->value<tmp->value) {
				heapArray[parent] = heapArray[child];
				getPointer(heapArray[parent]->index) = parent;
			} else {
				break;
			}
		}
		heapArray[parent] = tmp;
		getPointer(heapArray[parent]->index)=parent;
	}

	void percolateUp(size_t k) {
		size_t k_father;
		Indexable<T,C>* v = heapArray[k];
		k_father = k / 2; /* integer divsion to retrieve its parent */
		while (k_father > 0
				&& heapArray[k_father]->value>v->value) {
			heapArray[k] = heapArray[k_father];
			getPointer(heapArray[k]->index)=k;
			k = k_father;
			k_father = k / 2;
		}
		heapArray[k] = v;
		getPointer(heapArray[k]->index)=k;
	}
	void resize() {
		heapArray.resize(heapArray.size()*2,nullptr);
	}
};
typedef BinaryMinHeap<float, 2> BinaryMinHeap2f;
typedef BinaryMinHeap<float, 3> BinaryMinHeap3f;
typedef BinaryMinHeap<double, 2> BinaryMinHeap2d;
typedef BinaryMinHeap<double, 3> BinaryMinHeap3d;

};
#endif
