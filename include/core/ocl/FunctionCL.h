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

#ifndef INCLUDE_FUNCTIONCL
#define INCLUDE_FUNCTIONCL
#include <CL/cl.h>
#include <AlloyMath.h>
#include <AlloyVector.h>
#include <AlloyCommon.h>
#include <string>
#include <memory>
#include <exception>
#include <map>
namespace aly {
	class MemoryCL;
	class BufferCL;
	class BufferCLGL;
	class Image2dCL;
	class Image2dCLGL;
	class Image3dCL;
	class ProgramCL;
	enum class KernelAddressType {
		Global = (int) CL_KERNEL_ARG_ADDRESS_GLOBAL,
		Local = (int) CL_KERNEL_ARG_ADDRESS_LOCAL,
		Constant = (int) CL_KERNEL_ARG_ADDRESS_CONSTANT,
		Private = (int) CL_KERNEL_ARG_ADDRESS_PRIVATE
	};
	enum class KernelAccessType {
		ReadOnly = (int) CL_KERNEL_ARG_ACCESS_READ_ONLY,
		WriteOnly = (int) CL_KERNEL_ARG_ACCESS_WRITE_ONLY,
		ReadWrite = (int) CL_KERNEL_ARG_ACCESS_READ_WRITE,
		Unknown = (int) CL_KERNEL_ARG_ACCESS_NONE
	};
	struct ArgumentCL {
		std::string name;
		std::string type;
		KernelAddressType addressType;
		KernelAccessType accessType;
		cl_mem memory;
		ArgumentCL(const std::string& name = "", const std::string& type = "", const KernelAddressType& addressType = KernelAddressType::Global,
				const KernelAccessType& accessType = KernelAccessType::Unknown) :
				name(name), type(type), addressType(addressType), accessType(accessType), memory(nullptr) {
		}
	};
	size_t RoundToWorkgroup(size_t size, size_t workgroupSize);
	class FunctionCL {
	protected:
		std::string kernelName;
		std::string programName;
		cl_kernel kernel;
		std::vector<ArgumentCL> arguments;
		std::map<std::string, size_t> argMap;
		std::vector<size_t> globalSizes;
		std::vector<size_t> localSizes;
		std::vector<cl_mem> glObjects;
		std::vector<const void*> argValues;
		std::vector<size_t> argSizes;
		void setWorkGroupSize(size_t gx, size_t gy, size_t lx, size_t ly);
		void setWorkGroupSize(size_t gx, size_t lx);
		void execute(const std::vector<const void*>& args, std::vector<size_t>& sizes);
	public:
		const std::vector<ArgumentCL>& getArguments() const {
			return arguments;
		}
		size_t size() const {
			return arguments.size();
		}
		size_t getDimensions() const {
			return globalSizes.size();
		}
		std::string getName() const {
			return kernelName;
		}
		std::string getProgramName() const {
			return programName;
		}
		FunctionCL();
		FunctionCL(const ProgramCL &p, const std::string& name);
		FunctionCL(const ProgramCL &p, const std::string& name, const cl_kernel& kernel);
		~FunctionCL();

		FunctionCL& set(const std::string& name, cl_mem value);
		FunctionCL& set(const std::string& name, const MemoryCL& value);
		FunctionCL& set(const std::string& name, const BufferCL& value);
		FunctionCL& set(const std::string& name, const BufferCLGL& value);
		FunctionCL& set(const std::string& name, const Image2dCL& value);
		FunctionCL& set(const std::string& name, const Image2dCLGL& value);
		FunctionCL& set(const std::string& name, const Image3dCL& value);
		FunctionCL& set(const std::string& name, const int8_t& value);
		FunctionCL& set(const std::string& name, const int16_t& value);
		FunctionCL& set(const std::string& name, const int32_t& value);
		FunctionCL& set(const std::string& name, const uint8_t& value);
		FunctionCL& set(const std::string& name, const uint16_t& value);
		FunctionCL& set(const std::string& name, const uint32_t& value);
		FunctionCL& set(const std::string& name, const float& value);
		FunctionCL& set(const std::string& name, const double& value);
		template<class T, int M> FunctionCL& set(const std::string& name, const vec<T, M>& value) {
			auto pos = argMap.find(name);
			if (pos == argMap.end()) {
				throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
			}
			size_t idx = pos->second;
			argValues[idx] = (&value);
			argSizes[idx] = (sizeof(vec<T, M> ));
			return *this;
		}
		template<class T, int M> FunctionCL& set(const std::string& name, const box<T, M>& value) {
			auto pos = argMap.find(name);
			if (pos == argMap.end()) {
				throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
			}
			size_t idx = pos->second;
			argValues[idx] = (&value);
			argSizes[idx] = (sizeof(box<T, M> ));
			return *this;
		}
		template<class T, int M> FunctionCL& set(const std::string& name, const lineseg<T, M>& value) {
			auto pos = argMap.find(name);
			if (pos == argMap.end()) {
				throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
			}
			size_t idx = pos->second;
			argValues[idx] = (&value);
			argSizes[idx] = (sizeof(lineseg<T, M> ));
			return *this;
		}
		template<class T, int M, int N> FunctionCL& set(const std::string& name, const matrix<T, M, N>& value) {
			auto pos = argMap.find(name);
			if (pos == argMap.end()) {
				throw std::runtime_error(aly::MakeString() << "Could not set argument " << name << " in " << *this);
			}
			size_t idx = pos->second;
			argValues[idx] = (&value);
			argSizes[idx] = (sizeof(matrix<T, M, N> ));
			return *this;
		}
		void create(const ProgramCL &p, const std::string& name);
		void execute2D(size_t gx, size_t gy, size_t lx, size_t ly);
		void execute2D(size_t gx, size_t gy);
		void execute1D(size_t gx, size_t lx);
		void execute1D(size_t gx);
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const ArgumentCL& arg) {
		if (arg.accessType == KernelAccessType::Unknown) {
			ss << arg.addressType << " " << arg.type << " " << arg.name;
		} else {
			ss << arg.accessType << " " << arg.type << " " << arg.name;
		}
		return ss;
	}
	template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const FunctionCL& func) {
		ss << func.getName() << "(";
		size_t idx = 0;
		size_t sz = func.size();
		if (sz > 0) {
			for (ArgumentCL arg : func.getArguments()) {
				if (idx < sz - 1) {
					ss << arg << ", ";
				} else {
					ss << arg << ")";
				}
				idx++;
			}
		} else {
			ss << ")";
		}
		return ss;
	}
	template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const KernelAddressType& type) {
		switch (type) {
		case KernelAddressType::Global:
			return ss << "global";
		case KernelAddressType::Local:
			return ss << "local";
		case KernelAddressType::Constant:
			return ss << "constant";
		case KernelAddressType::Private:
			return ss << "private";
		}
		return ss;
	}
	template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const KernelAccessType& type) {
		switch (type) {
		case KernelAccessType::ReadOnly:
			return ss << "read_only";
		case KernelAccessType::WriteOnly:
			return ss << "write_only";
		case KernelAccessType::ReadWrite:
			return ss << "read_write";
		case KernelAccessType::Unknown:
			return ss << "unknown";
		}
		return ss;
	}
	typedef std::shared_ptr<FunctionCL> FunctionCLPtr;
}

#endif

