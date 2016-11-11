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
#include "AlloyVector.h"
#include "AlloyFileUtil.h"
#include <fstream>
#include "cereal/archives/json.hpp"
#include "cereal/archives/xml.hpp"

namespace aly {
template<class T, int C> void WriteVectorToFileInternal(const std::string& file, const Vector<T, C>& vector) {
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ofstream os(file);
		cereal::JSONOutputArchive archive(os);
		archive(cereal::make_nvp("vector", vector));
	} else if (ext == "xml") {
		std::ofstream os(file);
		cereal::XMLOutputArchive archive(os);
		archive(cereal::make_nvp("vector", vector));
	} else {
		uint64_t sz = vector.size();
		std::ofstream os(file, std::ios::binary);
		os.write((const char*) &sz, sizeof(uint64_t));
		os.write((const char*) vector.ptr(), (std::streamsize) (sz * vector.typeSize()));
	}
}
template<class T, int C> void ReadVectorFromFileInternal(const std::string& file, Vector<T, C>& vector) {
	std::string ext = GetFileExtension(file);
	if (ext == "json") {
		std::ifstream os(file);
		cereal::JSONInputArchive archive(os);
		archive(cereal::make_nvp("vector", vector));
	} else if (ext == "xml") {
		std::ifstream os(file);
		cereal::XMLInputArchive archive(os);
		archive(cereal::make_nvp("vector", vector));
	} else {
		std::ifstream os(file, std::ios::binary);
		uint64_t sz = 0;
		os.read((char*) &sz, sizeof(uint64_t));
		vector.resize(sz);
		os.read((char*) vector.ptr(), (std::streamsize) (sz * vector.typeSize()));
	}
}
void WriteVectorToFile(const std::string& file, const Vector<float, 4>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<float, 4>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<float, 3>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<float, 3>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<float, 2>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<float, 2>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<float, 1>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<float, 1>& vector) {
	ReadVectorFromFileInternal(file, vector);
}

void WriteVectorToFile(const std::string& file, const Vector<int, 4>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<int, 4>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<int, 3>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<int, 3>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<int, 2>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<int, 2>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<int, 1>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<int, 1>& vector) {
	ReadVectorFromFileInternal(file, vector);
}

void WriteVectorToFile(const std::string& file, const Vector<double, 4>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<double, 4>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<double, 3>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<double, 3>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<double, 2>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<double, 2>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
void WriteVectorToFile(const std::string& file, const Vector<double, 1>& vector) {
	WriteVectorToFileInternal(file, vector);
}
void ReadVectorFromFile(const std::string& file, Vector<double, 1>& vector) {
	ReadVectorFromFileInternal(file, vector);
}
}

