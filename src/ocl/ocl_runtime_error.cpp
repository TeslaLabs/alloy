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
#include <AlloyCommon.h>
#include <CL/cl.h>
namespace aly {
	void PrintCLError(int iErr) {
		if (iErr == CL_SUCCESS)
			return;
		std::cerr << "OpenCL Error:";
		switch (iErr) {
		case CL_INVALID_BUFFER_SIZE:
			std::cerr << "CL_INVALID_BUFFER_SIZE" << std::endl;
			break;
		case CL_INVALID_ARG_VALUE:
			std::cerr << "CL_INVALID_ARG_VALUE" << std::endl;
			break;
		case CL_DEVICE_NOT_FOUND:
			std::cerr << "CL_DEVICE_NOT_FOUND" << std::endl;
			break;
		case CL_BUILD_PROGRAM_FAILURE:
			std::cerr << "CL_BUILD_PROGRAM_FAILURE" << std::endl;
			break;
		case CL_INVALID_HOST_PTR:
			std::cerr << "CL_INVALID_HOST_PTR" << std::endl;
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			std::cerr << "CL_INVALID_WORK_GROUP_SIZE" << std::endl;
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			std::cerr << "CL_INVALID_GLOBAL_WORK_SIZE" << std::endl;
			break;
		case CL_INVALID_MEM_OBJECT:
			std::cerr << "CL_INVALID_MEM_OBJECT" << std::endl;
			break;
		case CL_INVALID_COMMAND_QUEUE:
			std::cerr << "CL_INVALID_COMMAND_QUEUE" << std::endl;
			break;
		case CL_INVALID_CONTEXT:
			std::cerr << "CL_INVALID_CONTEXT" << std::endl;
			break;
		case CL_INVALID_VALUE:
			std::cerr << "CL_INVALID_VALUE" << std::endl;
			break;
		case CL_INVALID_EVENT_WAIT_LIST:
			std::cerr << "CL_INVALID_EVENT_WAIT_LIST" << std::endl;
			break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			std::cerr << "CL_MISALIGNED_SUB_BUFFER_OFFSET" << std::endl;
			break;
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
			std::cerr << "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST" << std::endl;
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			std::cerr << "CL_MEM_OBJECT_ALLOCATION_FAILURE" << std::endl;
			break;
		case CL_OUT_OF_RESOURCES:
			std::cerr << "CL_OUT_OF_RESOURCES" << std::endl;
			break;
		case CL_INVALID_KERNEL:
			std::cerr << "CL_INVALID_KERNEL" << std::endl;
			break;
		case CL_INVALID_ARG_INDEX:
			std::cerr << "CL_INVALID_ARG_INDEX" << std::endl;
			break;
		case CL_OUT_OF_HOST_MEMORY:
			std::cerr << "CL_OUT_OF_HOST_MEMORY" << std::endl;
			break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
			std::cerr << "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR" << std::endl;
			break;
		case CL_INVALID_OPERATION:
			std::cerr << "CL_INVALID_OPERATION" << std::endl;
			break;
		case CL_INVALID_GL_OBJECT:
			std::cerr << "CL_INVALID_GL_OBJECT" << std::endl;
			break;
		case CL_INVALID_BINARY:
			std::cerr << "CL_INVALID_BINARY" << std::endl;
			break;
		case CL_INVALID_KERNEL_ARGS:
			std::cerr << "CL_INVALID_KERNEL_ARGS" << std::endl;
			break;
		case CL_INVALID_ARG_SIZE:
			std::cerr << "CL_INVALID_ARG_SIZE" << std::endl;
			break;
		default:
			std::cerr << "UNKNOWN OpenCL ERROR" << iErr << std::endl;
		}
	}
	ocl_runtime_error::ocl_runtime_error(std::string m, int errorCode) :
			std::runtime_error(m.c_str()), errorCode(errorCode) {
		std::string message;
		switch (errorCode) {
		case CL_DEVICE_NOT_FOUND:
			message = "CL_DEVICE_NOT_FOUND";
			break;
		case CL_DEVICE_NOT_AVAILABLE:
			message = "CL_DEVICE_NOT_AVAILABLE";
			break;
		case CL_COMPILER_NOT_AVAILABLE:
			message = "CL_COMPILER_NOT_AVAILABLE";
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			message = "CL_MEM_OBJECT_ALLOCATION_FAILURE";
			break;
		case CL_OUT_OF_RESOURCES:
			message = "CL_OUT_OF_RESOURCES";
			break;
		case CL_OUT_OF_HOST_MEMORY:
			message = "CL_OUT_OF_HOST_MEMORY";
			break;
		case CL_PROFILING_INFO_NOT_AVAILABLE:
			message = "CL_PROFILING_INFO_NOT_AVAILABLE";
			break;
		case CL_MEM_COPY_OVERLAP:
			message = "CL_MEM_COPY_OVERLAP";
			break;
		case CL_IMAGE_FORMAT_MISMATCH:
			message = "CL_IMAGE_FORMAT_MISMATCH";
			break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:
			message = "CL_IMAGE_FORMAT_NOT_SUPPORTED";
			break;
		case CL_BUILD_PROGRAM_FAILURE:
			message = "CL_BUILD_PROGRAM_FAILURE";
			break;
		case CL_MAP_FAILURE:
			message = "CL_MAP_FAILURE";
			break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			message = "CL_MISALIGNED_SUB_BUFFER_OFFSET";
			break;
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
			message = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
			break;
		case CL_COMPILE_PROGRAM_FAILURE:
			message = "CL_COMPILE_PROGRAM_FAILURE";
			break;
		case CL_LINKER_NOT_AVAILABLE:
			message = "CL_LINKER_NOT_AVAILABLE";
			break;
		case CL_LINK_PROGRAM_FAILURE:
			message = "CL_LINK_PROGRAM_FAILURE";
			break;
		case CL_DEVICE_PARTITION_FAILED:
			message = "CL_DEVICE_PARTITION_FAILED";
			break;
		case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
			message = "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
			break;
		case CL_INVALID_VALUE:
			message = "CL_INVALID_VALUE";
			break;
		case CL_INVALID_DEVICE_TYPE:
			message = "CL_INVALID_DEVICE_TYPE";
			break;
		case CL_INVALID_PLATFORM:
			message = "CL_INVALID_PLATFORM";
			break;
		case CL_INVALID_DEVICE:
			message = "CL_INVALID_DEVICE";
			break;
		case CL_INVALID_CONTEXT:
			message = "CL_INVALID_CONTEXT";
			break;
		case CL_INVALID_QUEUE_PROPERTIES:
			message = "CL_INVALID_QUEUE_PROPERTIES";
			break;
		case CL_INVALID_COMMAND_QUEUE:
			message = "CL_INVALID_COMMAND_QUEUE";
			break;
		case CL_INVALID_HOST_PTR:
			message = "CL_INVALID_HOST_PTR";
			break;
		case CL_INVALID_MEM_OBJECT:
			message = "CL_INVALID_MEM_OBJECT";
			break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
			message = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
			break;
		case CL_INVALID_IMAGE_SIZE:
			message = "CL_INVALID_IMAGE_SIZE";
			break;
		case CL_INVALID_SAMPLER:
			message = "CL_INVALID_SAMPLER";
			break;
		case CL_INVALID_BINARY:
			message = "CL_INVALID_BINARY";
			break;
		case CL_INVALID_BUILD_OPTIONS:
			message = "CL_INVALID_BUILD_OPTIONS";
			break;
		case CL_INVALID_PROGRAM:
			message = "CL_INVALID_PROGRAM";
			break;
		case CL_INVALID_PROGRAM_EXECUTABLE:
			message = "CL_INVALID_PROGRAM_EXECUTABLE";
			break;
		case CL_INVALID_KERNEL_NAME:
			message = "CL_INVALID_KERNEL_NAME";
			break;
		case CL_INVALID_KERNEL_DEFINITION:
			message = "CL_INVALID_KERNEL_DEFINITION";
			break;
		case CL_INVALID_KERNEL:
			message = "CL_INVALID_KERNEL";
			break;
		case CL_INVALID_ARG_INDEX:
			message = "CL_INVALID_ARG_INDEX";
			break;
		case CL_INVALID_ARG_VALUE:
			message = "CL_INVALID_ARG_VALUE";
			break;
		case CL_INVALID_ARG_SIZE:
			message = "CL_INVALID_ARG_SIZE";
			break;
		case CL_INVALID_KERNEL_ARGS:
			message = "CL_INVALID_KERNEL_ARGS";
			break;
		case CL_INVALID_WORK_DIMENSION:
			message = "CL_INVALID_WORK_DIMENSION";
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			message = "CL_INVALID_WORK_GROUP_SIZE";
			break;
		case CL_INVALID_WORK_ITEM_SIZE:
			message = "CL_INVALID_WORK_ITEM_SIZE";
			break;
		case CL_INVALID_GLOBAL_OFFSET:
			message = "CL_INVALID_GLOBAL_OFFSET";
			break;
		case CL_INVALID_EVENT_WAIT_LIST:
			message = "CL_INVALID_EVENT_WAIT_LIST";
			break;
		case CL_INVALID_EVENT:
			message = "CL_INVALID_EVENT";
			break;
		case CL_INVALID_OPERATION:
			message = "CL_INVALID_OPERATION";
			break;
		case CL_INVALID_GL_OBJECT:
			message = "CL_INVALID_GL_OBJECT";
			break;
		case CL_INVALID_BUFFER_SIZE:
			message = "CL_INVALID_BUFFER_SIZE";
			break;
		case CL_INVALID_MIP_LEVEL:
			message = "CL_INVALID_MIP_LEVEL";
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			message = "CL_INVALID_GLOBAL_WORK_SIZE";
			break;
		case CL_INVALID_PROPERTY:
			message = "CL_INVALID_PROPERTY";
			break;
		case CL_INVALID_IMAGE_DESCRIPTOR:
			message = "CL_INVALID_IMAGE_DESCRIPTOR";
			break;
		case CL_INVALID_COMPILER_OPTIONS:
			message = "CL_INVALID_COMPILER_OPTIONS";
			break;
		case CL_INVALID_LINKER_OPTIONS:
			message = "CL_INVALID_LINKER_OPTIONS";
			break;
		case CL_INVALID_DEVICE_PARTITION_COUNT:
			message = "CL_INVALID_DEVICE_PARTITION_COUNT";
			break;
		default:
			message = "UNKNOWN";
			break;
		}
		errorMessage = aly::MakeString() << "OpenCL Error [" << message << "]: " << m;
	}
	const char* ocl_runtime_error::what() const _GLIBCXX_USE_NOEXCEPT{
		return errorMessage.c_str();
	}

	ocl_runtime_error::~ocl_runtime_error(void) {
	}
}
