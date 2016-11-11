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
#include "AlloySparseBitSet.h"
#include <AlloyMath.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <fstream>
namespace aly {
void WriteSparseBitSetToFile(const std::string& file,
		const SparseBitSet& bitset) {
	std::ofstream os(file);
	cereal::BinaryOutputArchive ar(os);
	ar(cereal::make_nvp("sparse_bitset", bitset));
}
void ReadSparseBitSetFromFile(const std::string& file, SparseBitSet& bitset) {
	std::ifstream os(file);
	cereal::BinaryInputArchive ar(os);
	ar(cereal::make_nvp("sparse_bitset", bitset));
}
size_t SparseBitSet::size() const {
	return length;
}
size_t SparseBitSet::capacity() const {
	return bits.size() * STRIDE;
}

void SparseBitSet::set(size_t idx, bool val) {
	size_t offset = idx / STRIDE;
	if (val) {
		if (bits.find(offset) == bits.end()) {
			bits[offset] = std::bitset<STRIDE>();
		}
		bits[offset].set(idx % STRIDE, val);
		length = std::max(length, idx + 1);
	} else {
		if (bits.find(offset) != bits.end()) {
			bits[offset].set(idx % STRIDE, false);
		}
	}
}
bool SparseBitSet::test(size_t idx) {
	size_t offset = idx / STRIDE;
	if (bits.find(offset) != bits.end()) {
		return bits[offset][idx % STRIDE];
	} else {
		return false;
	}

}
void SparseBitSet::reset() {
	bits.clear();
}
void SparseBitSet::clear() {
	bits.clear();
	length = 0;
}

std::bitset<SparseBitSet::STRIDE>::reference SparseBitSet::operator[](
		size_t idx) {
	size_t offset = idx / STRIDE;
	if (bits.find(offset) == bits.end()) {
		bits[offset] = std::bitset<STRIDE>();
	}
	length = std::max(length, idx + 1);
	return bits[offset][idx % STRIDE];
}
std::vector<size_t> SparseBitSet::indexes() {
	std::vector<size_t> list;
	for (std::pair<const size_t, std::bitset<STRIDE>>& pr : bits) {
		std::bitset<STRIDE>& bit = pr.second;
		for (int i = 0; i < (int) STRIDE; i++) {
			bool val = bit[i];
			if (val) {
				list.push_back(pr.first * STRIDE + i);
			}
		}
	}
	return list;
}
size_t SparseBitSet::count() {
	size_t count = 0;
	for (std::pair<const size_t, std::bitset<STRIDE>>& pr : bits) {
		std::bitset<STRIDE>& bit = pr.second;
		for (int i = 0; i < (int) STRIDE; i++) {
			bool val = bit[i];
			if (val) {
				count++;
			}
		}
	}
	return count;
}
void SparseBitSet::resize(size_t sz) {
	this->length = sz;
}
SparseBitSet::SparseBitSet(size_t sz) :
		length(sz) {

}
} /* namespace bfx */
