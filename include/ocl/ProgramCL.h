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
#ifndef INCLUDE_PROGRAMCL
#define INCLUDE_PROGRAMCL
#include "FunctionCL.h"
#include <CL/cl.h>
#include <string>
#include <memory>
namespace aly {
	class ProgramCL {
	private:
		cl_program handle;
		std::string name;
		std::vector<std::shared_ptr<FunctionCL>> functions;
		std::map<std::string, size_t> funcMap;
	public:
		ProgramCL(const cl_program& program = nullptr, const std::string& name = "");
		cl_program getHandle() const {
			return handle;
		}
		size_t size() const {
			return functions.size();
		}
		std::shared_ptr<FunctionCL>& function(const std::string& func) {
			auto pos = funcMap.find(func);
			if (pos == funcMap.end()) {
				throw std::runtime_error(aly::MakeString() << "Could not find function " << func << " in program " << name);
			}
			return functions[pos->second];
		}

		const std::vector<std::shared_ptr<FunctionCL>>& getFunctions() const {
			return functions;
		}
		std::vector<std::shared_ptr<FunctionCL>>& getFunctions() {
			return functions;
		}
		void create(cl_program program);
		const std::string& getName() const {
			return name;
		}
		virtual ~ProgramCL(void) {
		}
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(std::basic_ostream<C, R> & ss, const ProgramCL& prog) {
		ss << "[Program=\"" << prog.getName() << "\", Functions=";
		size_t idx = 0;
		size_t sz = prog.size();
		if (sz > 0) {
			for (const std::shared_ptr<FunctionCL>& func : prog.getFunctions()) {
				if (idx < sz - 1) {
					ss << *func << ", ";
				} else {
					ss << *func << "]";
				}
				idx++;
			}
		} else {
			ss << "}]";
		}
		return ss;
	}
	typedef std::shared_ptr<ProgramCL> ProgramCLPtr;
}
#endif

