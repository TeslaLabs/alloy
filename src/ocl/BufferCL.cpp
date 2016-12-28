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


#include "ocl/BufferCL.h"
#include "ocl/ocl_runtime_error.h"
#include "ocl/ComputeCL.h"
namespace aly {
	BufferCL::BufferCL() :
			MemoryCL() {
	}
	void BufferCL::create(const cl_mem_flags& f, size_t bufferSize,void* data) {
		int err = 0;
		if (this->bufferSize != bufferSize) {
			if (buffer != nullptr)
				clReleaseMemObject(buffer);
			buffer = clCreateBuffer(CLContext(), f, bufferSize, data, &err);
			if (err != CL_SUCCESS)
				throw ocl_runtime_error("Could not create buffer.", err);
			this->bufferSize = bufferSize;
		}
	}
	void* BufferCL::mapRead(bool block, int wait_events_num, cl_event* wait_events) {
		int err = 0;
		void* ptr = clEnqueueMapBuffer(CLQueue(), buffer, block, CL_MAP_READ, 0, bufferSize, wait_events_num, wait_events, 0, &err);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not read buffer.", err);
		return ptr;
	}
	void* BufferCL::mapWrite(bool block, int wait_events_num, cl_event* wait_events) const {
		int err = 0;
		void* ptr = clEnqueueMapBuffer(CLQueue(), buffer, block, CL_MAP_WRITE, 0, bufferSize, wait_events_num, wait_events, 0, &err);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not write buffer.", err);
		return ptr;
	}
	void BufferCL::unmap(void* data) const {
		int err = clEnqueueUnmapMemObject(CLQueue(), buffer, data, 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not unmap buffer.", err);
	}
}
