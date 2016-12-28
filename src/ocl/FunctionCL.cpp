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

#include "ocl/FunctionCL.h"
#include "ocl/ocl_runtime_error.h"
#include "ocl/ComputeCL.h"
#include "ocl/BufferCL.h"
#include "ocl/BufferCLGL.h"
#include "ocl/Image2dCL.h"
#include "ocl/Image2dCLGL.h"
#include "ocl/Image3dCL.h"
#include "ocl/ProgramCL.h"
#include <CL/cl.h>
#include <iostream>
namespace aly {
	size_t RoundToWorkgroup(size_t size, size_t workgroupSize) {
		return (size % workgroupSize == 0) ? size : workgroupSize * (size / workgroupSize + 1);
	}
	FunctionCL::FunctionCL() :
			kernel(nullptr) {
	}
	FunctionCL::FunctionCL(const ProgramCL &p, const std::string& name) :
			FunctionCL() {
		int err = 0;
		kernel = clCreateKernel(p.getHandle(), name.c_str(), &err);
		if (err != CL_SUCCESS) {
			throw ocl_runtime_error(aly::MakeString() << "Could not find kernel " << name << " in program " << p.getName(), err);
		}
		create(p, name);
	}
	FunctionCL::FunctionCL(const ProgramCL &p, const std::string& name, const cl_kernel& kernel) :
			kernel(kernel) {
		create(p, name);
	}

	void FunctionCL::create(const ProgramCL& p, const std::string& name) {
		std::array<char, 1024> kName;
		int err = 0;
		cl_uint numArgs = 0;
		err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &numArgs, NULL);
		if (err != CL_SUCCESS) {
			throw ocl_runtime_error(aly::MakeString() << "Could not get number of kernel args for " << name << " in program " << p.getName(), err);
		}
		kName.fill(0);
		arguments.resize(numArgs);
		argSizes.resize(numArgs, 0);
		argValues.resize(numArgs, 0);
		cl_kernel_arg_address_qualifier addressQual = 0;
		cl_kernel_arg_access_qualifier accessQual = 0;
		for (cl_uint i = 0; i < numArgs; i++) {
			kName.fill(0);
			err = clGetKernelArgInfo(kernel, i, CL_KERNEL_ARG_NAME, sizeof(char) * kName.size(), kName.data(), NULL);
			if (err != CL_SUCCESS) {
				throw ocl_runtime_error(aly::MakeString() << "Could not get kernel argument name [" << i << "] in program " << p.getName(), err);
			}
			std::string argName(kName.data());
			kName.fill(0);
			err = clGetKernelArgInfo(kernel, i, CL_KERNEL_ARG_TYPE_NAME, sizeof(char) * kName.size(), kName.data(), NULL);
			if (err != CL_SUCCESS) {
				throw ocl_runtime_error(aly::MakeString() << "Could not get kernel argument type [" << i << "] in program " << p.getName(), err);
			}
			std::string argType(kName.data());
			err = clGetKernelArgInfo(kernel, i, CL_KERNEL_ARG_ADDRESS_QUALIFIER, sizeof(cl_kernel_arg_address_qualifier), &addressQual, NULL);
			if (err != CL_SUCCESS) {
				throw ocl_runtime_error(aly::MakeString() << "Could not get kernel address qualifier in program " << p.getName(), err);
			}
			err = clGetKernelArgInfo(kernel, i, CL_KERNEL_ARG_ACCESS_QUALIFIER, sizeof(cl_kernel_arg_access_qualifier), &accessQual, NULL);
			if (err != CL_SUCCESS) {
				throw ocl_runtime_error(aly::MakeString() << "Could not get kernel access qualifier in program " << p.getName(), err);
			}
			arguments[i] = ArgumentCL(argName, argType, static_cast<KernelAddressType>(addressQual), static_cast<KernelAccessType>(accessQual));
			argMap[argName] = (size_t) i;
		}
		kernelName = name;
		programName = p.getName();
	}

	FunctionCL& FunctionCL::set(const std::string& name, const MemoryCL& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;
		arguments[index].memory = value.getHandle();
		if (arguments[index].memory == nullptr)
			throw ocl_runtime_error("Buffer has not been allocated.");
		argValues[index] = (&arguments[index].memory);
		argSizes[index] = (sizeof(cl_mem));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const BufferCL& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		arguments[index].memory = value.getHandle();
		if (arguments[index].memory == nullptr)
			throw ocl_runtime_error("Buffer has not been allocated.");
		argValues[index] = (&arguments[index].memory);
		argSizes[index] = (sizeof(cl_mem));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const BufferCLGL& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;
		arguments[index].memory = value.getHandle();
		if (arguments[index].memory == nullptr)
			throw ocl_runtime_error("GL Buffer has not been allocated.");
		argValues[index] = (&arguments[index].memory);
		glObjects.push_back(arguments[index].memory);
		argSizes[index] = (sizeof(cl_mem));

		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, cl_mem tmp) {
		if (tmp == nullptr)
			throw ocl_runtime_error("Memory object has not been allocated.");
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;
		arguments[index].memory = tmp;
		argValues[index] = (&arguments[index].memory);
		argSizes[index] = (sizeof(cl_mem));
		return *this;
	}

	FunctionCL& FunctionCL::set(const std::string& name, const Image2dCL& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;
		arguments[index].memory = value.getHandle();
		if (arguments[index].memory == nullptr)
			throw ocl_runtime_error("Image2d has not been allocated.");
		argValues[index] = (&arguments[index].memory);
		argSizes[index] = (sizeof(cl_mem));
		if (value.getTextureId() >= 0)
			glObjects.push_back(arguments[index].memory);
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const Image2dCLGL& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		arguments[index].memory = value.getHandle();
		if (arguments[index].memory == nullptr)
			throw ocl_runtime_error("GL Image2d has not been allocated.");
		argValues[index] = (&arguments[index].memory);
		argSizes[index] = (sizeof(cl_mem));
		if (value.getTextureId() >= 0)
			glObjects.push_back(arguments[index].memory);
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const Image3dCL& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		arguments[index].memory = value.getHandle();
		if (arguments[index].memory == nullptr)
			throw ocl_runtime_error("Image3d has not been allocated.");
		argValues[index] = (&arguments[index].memory);
		argSizes[index] = (sizeof(cl_mem));
		return *this;
	}

	FunctionCL& FunctionCL::set(const std::string& name, const float& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(float));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const double& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(double));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const uint8_t& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(uint8_t));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const uint16_t& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(uint16_t));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const uint32_t& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(uint32_t));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const int8_t& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(uint8_t));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const int16_t& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(uint16_t));
		return *this;
	}
	FunctionCL& FunctionCL::set(const std::string& name, const int32_t& value) {
		auto pos = argMap.find(name);
		if (pos == argMap.end()) {
			throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
		}
		size_t index = pos->second;

		argValues[index] = (&value);
		argSizes[index] = (sizeof(uint32_t));
		return *this;
	}
	void FunctionCL::setWorkGroupSize(size_t gx, size_t lx) {
		globalSizes.resize(1);
		globalSizes[0] = RoundToWorkgroup(gx, lx);
		localSizes.resize(1);
		localSizes[0] = lx;

	}
	void FunctionCL::setWorkGroupSize(size_t gx, size_t gy, size_t lx, size_t ly) {
		globalSizes.resize(2);
		localSizes.resize(2);
		globalSizes[0] = RoundToWorkgroup(gx, lx);
		globalSizes[1] = RoundToWorkgroup(gy, gy);
		localSizes[0] = lx;
		localSizes[1] = ly;
	}
	void FunctionCL::execute2D(size_t gx, size_t gy, size_t lx, size_t ly) {
		setWorkGroupSize(gx, gy, lx, ly);
		if (!glObjects.empty()) {
			if (CLInstance()->lockGL(glObjects)) {
				execute(argValues, argSizes);
				CLInstance()->unlockGL(glObjects);
			}
		} else {
			execute(argValues, argSizes);
		}
	}
	void FunctionCL::execute2D(size_t gx, size_t gy) {
		globalSizes.resize(2);
		globalSizes[0] = gx;
		globalSizes[1] = gy;
		localSizes.clear();
		bool locked = false;
		try {
			if (!glObjects.empty()) {
				if (CLInstance()->lockGL(glObjects)) {
					locked = true;
					execute(argValues, argSizes);
					CLInstance()->unlockGL(glObjects);
				}
			} else {
				execute(argValues, argSizes);
			}
		} catch (const ocl_runtime_error& e) {
			if (locked)
				CLInstance()->unlockGL(glObjects);
			throw e;
		}
	}
	void FunctionCL::execute1D(size_t gx, size_t lx) {
		setWorkGroupSize(gx, lx);
		bool locked = false;
		try {
			if (!glObjects.empty()) {
				if (CLInstance()->lockGL(glObjects)) {
					locked = true;
					execute(argValues, argSizes);
					CLInstance()->unlockGL(glObjects);
				}
			} else {
				execute(argValues, argSizes);
			}
		} catch (const ocl_runtime_error& e) {
			if (locked)
				CLInstance()->unlockGL(glObjects);
			throw e;
		}
	}
	void FunctionCL::execute1D(size_t gx) {
		globalSizes.resize(1);
		localSizes.clear();
		globalSizes[0] = gx;
		bool locked = false;
		try {
			if (!glObjects.empty()) {
				if (CLInstance()->lockGL(glObjects)) {
					locked = true;
					execute(argValues, argSizes);
					CLInstance()->unlockGL(glObjects);
				}
			} else {
				execute(argValues, argSizes);
			}
		} catch (const ocl_runtime_error& e) {
			if (locked)
				CLInstance()->unlockGL(glObjects);
			throw e;
		}
	}
	void FunctionCL::execute(const std::vector<const void*>& args, std::vector<size_t>& sizes) {
		cl_int err = -1;
		int dims = globalSizes.size();
		if (args.size() == sizes.size()) {
			for (cl_uint i = 0; i < args.size(); i++) {
				err = clSetKernelArg(kernel, i, sizes[i], args[i]);
				if (CL_SUCCESS != err) {
					throw ocl_runtime_error(
							aly::MakeString() << "Function " << kernelName << " could not set argument [" << (i + 1) << " / [" << args.size() << "].", err);
				}
			}
			if (localSizes.empty()) {
				err = clEnqueueNDRangeKernel(CLQueue(), kernel, dims, nullptr, (const size_t*) &globalSizes[0], nullptr, 0, nullptr, nullptr);
			} else {
				err = clEnqueueNDRangeKernel(CLQueue(), kernel, dims, nullptr, (const size_t*) &globalSizes[0], (const size_t*) &localSizes[0], 0, nullptr,
						nullptr);
			}
			if (err != CL_SUCCESS) {
				throw ocl_runtime_error(aly::MakeString() << "Function " << kernelName << " failed.", err);
			}
			glObjects.clear();
		} else {
			throw ocl_runtime_error(
					aly::MakeString() << "Function " << kernelName << " value and size arguments don't align " << args.size() << "/" << sizes.size() << ".");
			glObjects.clear();
		}
	}
	FunctionCL::~FunctionCL() {
		if (kernel != nullptr)
			clReleaseKernel(kernel);
	}
}
