/*
 * Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef INCLUDE_MEMORYCL
#define INCLUDE_MEMORYCL
#include <CL/cl.h>
#include <vector>
namespace aly {
	class MemoryCL {
	protected:
		MemoryCL();
		size_t bufferSize;
		cl_mem buffer;
		virtual void read(void* data, size_t sz = 0, bool block = true) const;
		virtual void write(const void* data, size_t sz = 0, bool block = true);
	public:
		size_t size() const {
			return bufferSize;
		}
		cl_mem getHandle() const {
			return buffer;
		}
		template<typename T> void read(std::vector<T>& vec, bool block = true) const {
			read(vec.data(), sizeof(T) * vec.size(), block);
		}
		template<typename T> void write(const std::vector<T>& vec, bool block = true) {
			write(vec.data(), sizeof(T) * vec.size(), block);
		}
		virtual void clear();
		virtual void copyFrom(const MemoryCL& src);
		virtual void release();
		virtual ~MemoryCL();
	};
}
#endif

