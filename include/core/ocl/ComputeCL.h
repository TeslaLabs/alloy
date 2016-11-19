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

#ifndef INCLUDE_CLCOMPUTE
#define INCLUDE_CLCOMPUTE
#include <vector>
#include <string>
#include <memory>
#include <CL/cl.h>
void SANITY_CHECK_OPENCL();
namespace aly {
	class ProgramCL;
	class ComputeCL {
	public:
		enum class Device {
			Default = (int) CL_DEVICE_TYPE_DEFAULT,
			Accelerator = (int) CL_DEVICE_TYPE_ACCELERATOR,
			CPU = (int) CL_DEVICE_TYPE_CPU,
			GPU = (int) CL_DEVICE_TYPE_GPU,
			ALL = (int) CL_DEVICE_TYPE_ALL
		};
	private:
		static std::shared_ptr<ComputeCL> computeInstance;
		cl_platform_id platformId;
		cl_device_id deviceId;
		cl_context context;
		cl_command_queue commandQueue;
		bool initialized;
		Device deviceType;
		std::vector<cl_platform_id> platforms;
		std::string buildOptions;
		void enumeratePlatforms();
		std::vector<cl_device_id> enumerateDevices(cl_platform_id id) const;
		void setDevice(const Device& cd, const std::string& vendor = "");
		std::string getBuildLog(cl_program program) const;
		std::vector<std::string> computeHashCodes(const std::vector<std::string>& fileNames) const;
		ComputeCL();
	public:
		~ComputeCL();
		std::string getPlatformInfo(const cl_platform_id& id) const;
		std::string getDeviceInfo(const cl_device_id& id) const;
		std::string getPlatformInfo() const;
		static std::shared_ptr<ComputeCL> Instance();
		static void Destroy();
		static void Flush();
		static int Finish();
		void flush() const;
		int finish() const;
		cl_device_id getDevice() const {
			return deviceId;
		}
		cl_context getContext() const {
			return context;
		}
		cl_command_queue getCommandQueue() const {
			return commandQueue;
		}
		bool isInitialized() const {
			return initialized;
		}
		bool hasExtension(cl_device_id id, const std::string& name) const;
		bool isVendor(cl_platform_id id, const std::string& vendor) const;
		bool isVendor(cl_device_id id, const std::string& vendor) const;
		void initialize(const std::string& devicename = "", const ComputeCL::Device& device = ComputeCL::Device::GPU);
		std::shared_ptr<ProgramCL> compileFiles(const std::vector<std::string>& fileNames);
		std::shared_ptr<ProgramCL> compileSources(const std::vector<std::string>& srcCodes);
		std::shared_ptr<ProgramCL> compileFile(const std::string& fileName);
		std::shared_ptr<ProgramCL> compileSource(const std::string& srcCode);
		std::shared_ptr<ProgramCL> compileFiles(const std::string& name, const std::vector<std::string>& fileNames);
		bool lockGL(std::vector<cl_mem>& memobjs);
		bool unlockGL(std::vector<cl_mem>& memobjs);
		bool lockGL(std::initializer_list<cl_mem>& memobjs);
		bool unlockGL(std::initializer_list<cl_mem>& memobjs);
	};
	inline std::shared_ptr<ComputeCL> CLInstance() {
		return ComputeCL::Instance();
	}
	std::shared_ptr<ProgramCL> CLCompile(const std::string& name, const std::vector<std::string>& fileNames);
	std::shared_ptr<ProgramCL> CLCompile(const std::string& name, const std::initializer_list<std::string>& fileNames);
	std::shared_ptr<ProgramCL> CLCompile(const std::string& fileName);
	inline cl_command_queue CLQueue() {
		return ComputeCL::Instance()->getCommandQueue();
	}
	inline cl_context CLContext() {
		return ComputeCL::Instance()->getContext();
	}
	void CLInitialize(const std::string& vendor = "", const ComputeCL::Device& device=ComputeCL::Device::GPU);
	void CLDestroy();
	template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const ComputeCL::Device& type) {
		switch (type) {
		case ComputeCL::Device::CPU:
			return ss << "CPU";
		case ComputeCL::Device::GPU:
			return ss << "GPU";
		case ComputeCL::Device::ALL:
			return ss << "ALL";
		case ComputeCL::Device::Accelerator:
			return ss << "Accelerator";
		case ComputeCL::Device::Default:
			return ss << "Default";
		default:
			return ss << "Unknown";
		}
		return ss;
	}
}
#endif

