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

#ifndef ALLOYFILEUTIL_H_
#define ALLOYFILEUTIL_H_
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include "sha1.h"
#include "sha2.h"
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS // suppress warnings about fopen()
#endif
namespace aly {
#if defined(__APPLE__) && defined(__MACH__)
	#define ALY_APPLE
#endif
#if defined(WIN32) || defined(_WIN32)
#define ALY_PATH_SEPARATOR std::string("\\")
#define ALY_WINDOWS
#else
#define ALY_PATH_SEPARATOR std::string("/")
#endif
	bool SANITY_CHECK_FILE_IO();
	enum class FileType {
		Unknown, File, Directory
	};
	enum class FileAttribute {
		Compressed, Hidden
	};
	enum class HashMethod {
		SHA1 = 1, SHA224 = 224, SHA256 = 256, SHA384 = 384, SHA512 = 512
	};
	struct FileDescription {
		std::string fileLocation;
		FileType fileType;
		size_t fileSize;
		bool readOnly;
		std::time_t creationTime;
		std::time_t lastAccessTime;
		std::time_t lastModifiedTime;
		FileDescription(const std::string& fileLocation, FileType fileType,
			size_t fileSize, bool readOnly, std::time_t creationTime,
			std::time_t lastAccessTime, std::time_t lastModifiedTime) :
			fileLocation(fileLocation), fileType(fileType), fileSize(fileSize), readOnly(
				readOnly), creationTime(creationTime), lastAccessTime(
					lastAccessTime), lastModifiedTime(lastModifiedTime) {

		}
		FileDescription() :
			fileLocation(""), fileType(FileType::Unknown), fileSize(0), readOnly(
				false), creationTime(0), lastAccessTime(0), lastModifiedTime(
					0) {
		}
		bool operator<(const FileDescription& fd) const {
			return std::lexicographical_compare(fileLocation.begin(),
				fileLocation.end(), fd.fileLocation.begin(),
				fd.fileLocation.end());
		}
		bool operator<(FileDescription& fd) {
			return std::lexicographical_compare(fileLocation.begin(),
				fileLocation.end(), fd.fileLocation.begin(),
				fd.fileLocation.end());
		}
	};
	FileDescription GetFileDescription(const std::string& fileLocation);
	size_t GetFileSize(const std::string& fileName);
	std::string GetFileExtension(const std::string& fileName);
	std::string GetFileWithoutExtension(const std::string& file);
	std::string GetFileNameWithoutExtension(const std::string& file);
	std::string GetParentDirectory(const std::string& dir);
	std::string ReplaceFileExtension(const std::string& file,
		const std::string& ext);
	std::string GetFileDirectoryPath(const std::string& file);
	std::string GetFileName(const std::string& file);
	std::string RemoveTrailingSlash(const std::string& file);
	bool RemoveFile(const std::string& file);
	bool RemoveDirectory(const std::string& dir);
	bool RemoveDirectoryRecursive(const std::string& dir);
	std::string ConcatPath(const std::string& dir, const std::string& file);
	std::vector<std::string> GetDirectoryFileListing(const std::string& dirName,
		const std::string& ext = "", const std::string& mask = "");
	std::vector<FileDescription> GetDirectoryDescriptionListing(
		const std::string& dirName);
	std::vector<std::string> GetDirectoryListing(const std::string& dirName);
	std::string ReadTextFile(const std::string& str);
	std::vector<char> ReadBinaryFile(const std::string& str);
	void WriteTextFile(const std::string& file,const std::string& str);
	void WriteBinaryFile(const std::string& str, const std::vector<char>& data);
	void WriteBinaryFile(const std::string& str, const char* data, size_t size);
	bool FileExists(const std::string& name);
	bool IsDirectory(const std::string& file);
	bool IsFile(const std::string& file);
	std::string CodePointToUTF8(int cp);
	std::vector<std::string> SplitPath(const std::string& file);
	std::string Concat(const std::vector<std::string>& list);
	std::string FormatTime(const std::time_t& time);
	std::string FormatDate(const std::time_t& time);
	std::string FormatDateAndTime(const std::time_t& time);
	std::string FormatSize(size_t size);
	std::string GetCurrentWorkingDirectory();
	std::string GetHomeDirectory();
	std::string GetDownloadsDirectory();
	std::string GetDesktopDirectory();
	std::string GetDocumentsDirectory();
	std::string GetExecutableDirectory();
	std::string GetUserNameString();
	bool MakeDirectory(const std::string& dir);
	std::vector<std::string> GetDrives();
	static inline bool is_base64(unsigned char c) {
		return (isalnum(c) || (c == '+') || (c == '-'));
	}

	template<class T> std::string EncodeBase64(const std::vector<T>& in, bool pad =
		true) {
		int i = 0;
		int j = 0;
		static const std::string base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];
		size_t idx = 0;
		const T* dataPtr = in.data();
		const uint8_t* bytes_to_encode = (const uint8_t*)(dataPtr);
		size_t in_len = in.size() * sizeof(T);
		std::stringstream bufferOut;
		while (in_len--) {
			char_array_3[i++] = bytes_to_encode[idx++];
			if (i == 3) {
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
					+ ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
					+ ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					bufferOut << base64_chars[char_array_4[i]];
				i = 0;
			}
		}
		if (i) {
			for (j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
				+ ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
				+ ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
				bufferOut << base64_chars[char_array_4[j]];

			if (pad) {
				while ((i++ < 3))
					bufferOut << "=";
			}
		}
		return bufferOut.str();
	}
	template<class T> std::string HashCode(std::vector<T> data, HashMethod method =
		HashMethod::SHA256) {
		std::string str = EncodeBase64(data);
		std::vector<unsigned char> hashOut;
		std::string hashCode = "";
		switch (method) {
		case HashMethod::SHA1:
			hashCode = sha1(str);
			break;
		case HashMethod::SHA224:
			hashOut.resize(SHA224_DIGEST_SIZE);
			sha224((unsigned char *)str.c_str(), str.size(), hashOut.data());
			hashCode = EncodeBase64(hashOut, false);
			break;
		case HashMethod::SHA256:
			hashOut.resize(SHA256_DIGEST_SIZE);
			sha256((unsigned char *)str.c_str(), str.size(), hashOut.data());
			hashCode = EncodeBase64(hashOut, false);
			break;
		case HashMethod::SHA384:
			hashOut.resize(SHA384_DIGEST_SIZE);
			sha384((unsigned char *)str.c_str(), str.size(), hashOut.data());
			hashCode = EncodeBase64(hashOut, false);
			break;
		case HashMethod::SHA512:
			hashOut.resize(SHA512_DIGEST_SIZE);
			sha512((unsigned char *)str.c_str(), str.size(), hashOut.data());
			hashCode = EncodeBase64(hashOut, false);
			break;
		}
		return hashCode;
	}
	template<class T> void DecodeBase64(const std::string& encoded_string,
		std::vector<T>& out) {
		static const std::string base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
		size_t in_len = encoded_string.size();
		size_t i = 0;
		size_t j = 0;
		size_t in_ = 0;
		unsigned char char_array_4[4], char_array_3[3];
		std::vector<uint8_t> bufferOut;
		while (in_len-- && (encoded_string[in_] != '=')
			&& is_base64(encoded_string[in_])) {
			char_array_4[i++] = encoded_string[in_];
			in_++;
			if (i == 4) {
				for (i = 0; i < 4; i++)
					char_array_4[i] = static_cast<unsigned char>(base64_chars.find(
						char_array_4[i]));

				char_array_3[0] = (char_array_4[0] << 2)
					+ ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4)
					+ ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					bufferOut.push_back(char_array_3[i]);
				i = 0;
			}
		}

		if (i) {
			for (j = i; j < 4; j++)
				char_array_4[j] = 0;
			for (j = 0; j < 4; j++)
				char_array_4[j] = static_cast<unsigned char>(base64_chars.find(
					char_array_4[j]));
			char_array_3[0] = (char_array_4[0] << 2)
				+ ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4)
				+ ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
			for (j = 0; (j < i - 1); j++)
				bufferOut.push_back(char_array_3[j]);
		}
		out.resize(bufferOut.size() / sizeof(T));
		std::memcpy(out.data(), bufferOut.data(), sizeof(T) * out.size());
	}
	std::vector<std::string> AutoComplete(const std::string& str,
		const std::vector<std::string>& list, int maxSuggestions = -1);

	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const FileType& type) {
		switch (type) {
		case FileType::Directory:
			ss << "Directory";
			break;
		case FileType::File:
			ss << "File";
			break;
		default:
			ss << "Unknown";
		}
		return ss;
	}

	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const FileDescription& fd) {
		ss << GetFileName(fd.fileLocation) << " " << fd.fileType << " "
			<< ((fd.readOnly) ? "Read/Write" : "Read Only") << " "
			<< FormatSize(fd.fileSize) << " Created: "
			<< FormatDate(fd.creationTime) << " Last Access: "
			<< FormatDate(fd.lastAccessTime) << " Last Modified: "
			<< FormatDate(fd.lastModifiedTime);
		return ss;
	}
}
#endif /* ALLOYFILEUTIL_H_ */
