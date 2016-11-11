/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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

#ifndef ALLOYCOMMON_H_
#define ALLOYCOMMON_H_
#include "tinyformat.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ios>
#include <vector>
#include <locale>
#include <memory>


namespace aly {
	//GCC doesn't have this for some reason.
	template <typename T, typename... Args> inline std::shared_ptr<T> MakeShared(Args&&... args)
	{
		return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
	}
	template <typename T, typename... Args> inline std::unique_ptr<T> MakeUnique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
	struct MakeString {
		std::ostringstream ss;
		operator std::string() const {
			return ss.str();
		}
		template<class T> MakeString & operator <<(const T & val) {
			ss.setf(std::ios::fixed, std::ios::floatfield);
			ss << val;
			return *this;
		}
	};
	bool Contains(const std::string& str, const std::string& pattern);
	int Contains(std::string& str,std::vector<std::string> tokens);

	bool BeginsWith(const std::string& str, const std::string& pattern);
	bool EndsWith(const std::string& str, const std::string& pattern);
	std::string ToLower(const std::string& str);
	std::string ToUpper(const std::string& str);
	bool ContainsIgnoreCase(const std::string& str, const std::string& pattern);
	std::vector<std::string> Split(const std::string& str, char delim);
	std::vector<std::string> Tokenize(const std::string& str);
}

#endif /* ALLOYCOMMON_H_ */
