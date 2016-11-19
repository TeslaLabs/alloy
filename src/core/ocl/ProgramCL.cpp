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
#include "ocl/ProgramCL.h"
#include <iostream>
#include <vector>
#include <AlloyCommon.h>
namespace aly {
	ProgramCL::ProgramCL(const cl_program& program, const std::string& name) :
			handle(program), name(name) {
		if (program != nullptr) {
			create(program);
		}
	}
	void ProgramCL::create(cl_program program) {
		handle = program;
		cl_uint numKernels = 0;
		int err = clCreateKernelsInProgram(program, 0, NULL, &numKernels);
		if (err != CL_SUCCESS) {
			throw ocl_runtime_error(aly::MakeString() << "Could not create all kernels in program " << name, err);
		}
		std::vector<cl_kernel> kernels(numKernels, 0);
		err = clCreateKernelsInProgram(program, numKernels, kernels.data(), NULL);
		if (err != CL_SUCCESS) {
			throw ocl_runtime_error(aly::MakeString() << "Could not get number of kernels in program " << name, err);
		}
		std::array<char, 1024> kName;
		functions.clear();
		funcMap.clear();
		for (cl_kernel kernel : kernels) {
			kName.fill(0);
			err = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, sizeof(char) * kName.size(), kName.data(), NULL);
			std::string str(kName.data());
			if (err != CL_SUCCESS) {
				throw ocl_runtime_error(aly::MakeString() << "Could not get kernel name in program " << name, err);
			}
			funcMap[str] = functions.size();
			functions.push_back(std::shared_ptr<FunctionCL>(new FunctionCL(*this, str, kernel)));
		}
	}
}
