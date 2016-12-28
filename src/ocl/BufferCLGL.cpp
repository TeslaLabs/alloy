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
#include "ocl/BufferCLGL.h"
#include "ocl/ComputeCL.h"
#include <CL/cl_gl.h>
namespace aly {
	BufferCLGL::BufferCLGL() :
			MemoryCL(), id(-1) {
	}
	void BufferCLGL::create(cl_mem_flags f, unsigned int glBuf, size_t sz) {
		int err = 0;
		bufferSize = sz;
		id = glBuf;
		if (buffer != nullptr)
			clReleaseMemObject(buffer);
		buffer = clCreateFromGLBuffer(CLContext(), f, glBuf, &err);
		if (CL_SUCCESS != err)
			throw ocl_runtime_error("Could not create CL/GL buffer.", err);
	}
}
