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
#include "AlloyCursorLocator.h"
#include "AlloyUI.h"
namespace aly {
void CursorLocator::reset(int2 viewportDims) {
	//std::lock_guard<std::mutex> lockMe(lock);
	cellSize.x = std::max((pixel) 1, viewportDims.x / (pixel) ROWS);
	cellSize.y = std::max((pixel) 1, viewportDims.y / (pixel) COLS);
	for (int j = 0; j < COLS; j++) {
		for (int i = 0; i < ROWS; i++) {
			grid[i][j].clear();
		}
	}
}
void CursorLocator::add(Region* region) {
	box2px bounds = region->getCursorBounds();
	if (bounds.dimensions.x * bounds.dimensions.y == 0)
		return;
	int2 start = clamp(int2(bounds.position / cellSize), lowerBounds,
			upperBounds);
	int2 end = clamp(int2((bounds.position + bounds.dimensions) / cellSize),
			lowerBounds, upperBounds);
	for (int j = (int) start.y; j <= (int) end.y; j++) {
		for (int i = (int) start.x; i <= (int) end.x; i++) {
			grid[i][j].push_front(region);
		}
	}
}
Region* CursorLocator::locate(const pixel2& cursor) const {
	if (cursor.x < 0 || cursor.y < 0)
		return nullptr;
	int2 query = clamp(int2(cursor / cellSize), lowerBounds, upperBounds);
	for (Region* region : grid[(int) query.x][(int) query.y]) {
		Region* over = region->locate(cursor);
		if (over != nullptr)
			return over;
	}
	return nullptr;
}

}

