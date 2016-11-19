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


#include "ocl/ocl_runtime_error.h"
#include "ocl/MemoryCL.h"
#include "ocl/ComputeCL.h"
#include <string>
#include <iostream>
namespace aly {
	MemoryCL::MemoryCL() :
			bufferSize(0), buffer(nullptr) {
	}
	void MemoryCL::clear() {
		if (buffer == nullptr)
			return;
		const char ZERO = 0;
		int err = clEnqueueFillBuffer(CLQueue(), buffer, &ZERO, sizeof(char), 0, size(), 0, NULL, NULL);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not clear memory.", err);
	}
	void MemoryCL::release() {
		if (buffer == nullptr)
			clReleaseMemObject(buffer);
		buffer = nullptr;
		bufferSize = 0;
	}
	MemoryCL::~MemoryCL() {
		release();
	}
	void MemoryCL::read(void* data, size_t sz, bool block) const {
		if (sz != 0)
			sz = bufferSize;
		int err = clEnqueueReadBuffer(CLQueue(), buffer, (block) ? CL_TRUE : CL_FALSE, 0, sz, data, 0, 0, NULL);
		if (err != CL_SUCCESS) {
			throw ocl_runtime_error("Could not read memory.", err);
		}
	}
	void MemoryCL::write(const void* data, size_t sz, bool block) {
		if (sz != 0)
			sz = bufferSize;
		int err = clEnqueueWriteBuffer(CLQueue(), buffer, (block) ? CL_TRUE : CL_FALSE, 0, sz, data, 0, NULL, NULL);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not write.", err);
	}
	void MemoryCL::copyFrom(const MemoryCL& src) {
		int err = clEnqueueCopyBuffer(CLQueue(), src.buffer, buffer, 0, 0, bufferSize, 0, NULL, NULL);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not copy.", err);
	}
}
