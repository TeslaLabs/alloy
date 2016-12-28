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

#ifndef INCLUDE_BUFFERCL
#define INCLUDE_BUFFERCL
#include "MemoryCL.h"
#include <AlloyVector.h>
namespace aly {
	class BufferCL: public MemoryCL {
	public:
		BufferCL();
		void create(const cl_mem_flags& f, size_t bufferSize,void* data = nullptr);
		void create(size_t bufferSize,void* data = nullptr) {
			create(CL_MEM_READ_WRITE, bufferSize, data);
		}
		template<class T, int M> void create(const cl_mem_flags& f, Vector<T, M>& vec) {
			create(f, vec.size() * vec.typeSize(), vec.ptr());
		}
		void* mapRead(bool block = true, int wait_events_num = 0, cl_event* wait_events = nullptr);
		void* mapWrite(bool block = true, int wait_events_num = 0, cl_event* wait_events = nullptr) const;
		void unmap(void* data) const;
		template<class T, int M> BufferCL(const Vector<T, M>& vec, const cl_mem_flags& f = CL_MEM_READ_WRITE) :
				BufferCL() {
			create(f, vec);
		}
		template<class T, int M> void read(Vector<T, M>& vec, bool block = true) const {
			MemoryCL::read(vec.data, block);
		}
		template<class T, int M> void write(const Vector<T, M>& vec, bool block = true) {
			if(buffer==nullptr){
				create(CL_MEM_READ_WRITE, vec.size() * vec.typeSize());
			}
			MemoryCL::write(vec.data, block);
		}
	};
}
#endif

