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

#ifndef ALLOYLOCATOR_H_
#define ALLOYLOCATOR_H_

#include "AlloyMath.h"
#include "AlloyUnits.h"
#include <list>
#include <mutex>
namespace aly {
class Region;
class CursorLocator {
	static const int ROWS = 32;
	static const int COLS = 18;
	std::list<Region*> grid[ROWS][COLS];
	const int2 lowerBounds = int2(0, 0);
	const int2 upperBounds = int2(ROWS - 1, COLS - 1);
	pixel2 cellSize;
	//std::mutex lock;
public:
	CursorLocator() {
	}
	void reset(int2 viewportDims);
	void add(Region* region);
	Region* locate(const pixel2& cursor) const;
};
}
#endif /* ALLOYLOCATOR_H_ */
