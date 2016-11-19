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
#include "ocl/Image2dCLGL.h"
#include "ocl/ComputeCL.h"
#include  <GLTexture.h>
//#include <GL/glew.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <AlloyCommon.h>
namespace aly {
	Image2dCLGL::Image2dCLGL() :
			Image2dCL(), imageType(ImageType::UNKNOWN) {
	}

	Image2dCLGL::~Image2dCLGL() {
		if (buffer != nullptr)
			clReleaseMemObject(buffer);
		buffer = nullptr;
		bufferSize = 0;
	}
	int Image2dCLGL::create(cl_mem_flags f, unsigned int glId, const ImageType& type, int w, int h, int c, int bytesPerChannel) {
		int err = -1;
		width = w;
		height = h;
		channels = c;
		imageType = type;
		typeSize = bytesPerChannel;
		bufferSize = (size_t) height * (size_t) width * (size_t) channels * (size_t) typeSize;
		textureId = glId;
		if (buffer != nullptr)
			clReleaseMemObject(buffer);
		glBindTexture(GL_TEXTURE_2D, textureId);
		buffer = clCreateFromGLTexture(CLContext(), f, GL_TEXTURE_2D, 0, textureId, &err);
		if (err != CL_SUCCESS) {
			throw ocl_runtime_error(
					aly::MakeString() << "Could not create 2D image [w=" << width << ",h=" << height << ",c=" << channels << ",b=" << typeSize << "].", err);
		}
		regions[0] = width;
		regions[1] = height;
		regions[2] = 1;
		glBindTexture(GL_TEXTURE_2D, 0);
		return textureId;
	}

	int Image2dCLGL::create(cl_mem_flags f, const ImageType& type, int w, int h, int c, int bytesPerChannel) {
		textureId = -1;
		int err = -1;
		height = h;
		width = w;
		channels = c;
		imageType = type;
		typeSize = bytesPerChannel;
		bufferSize = (size_t) w * (size_t) h * (size_t) c * (size_t) bytesPerChannel;
		glGenTextures(1, (unsigned int*) &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLuint internalFormat = 0;
		GLuint externalFormat = 0;
		GLuint dataType = 0;
		switch (type) {
		case ImageType::FLOAT:
			if (channels == 4) {
				internalFormat = GL_RGBA32F;
				externalFormat = GL_RGBA;
				dataType = GL_FLOAT;
			} else if (channels == 3) {
				internalFormat = GL_RGB32F;
				externalFormat = GL_RGB;
				dataType = GL_FLOAT;
			} else if (channels == 1) {
				internalFormat = GL_R32F;
				externalFormat = GL_R;
				dataType = GL_FLOAT;
			} else {
				throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
			}
			break;
		case ImageType::UBYTE:
			if (channels == 4) {
				internalFormat = GL_RGBA;
				externalFormat = GL_RGBA;
				dataType = GL_UNSIGNED_BYTE;
			} else if (channels == 3) {
				internalFormat = GL_RGB;
				externalFormat = GL_RGB;
				dataType = GL_UNSIGNED_BYTE;
			} else if (channels == 1) {
				internalFormat = GL_R;
				externalFormat = GL_R;
				dataType = GL_UNSIGNED_BYTE;
			} else {
				throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
			}
			break;
		case ImageType::USHORT:
			if (channels == 4) {
				internalFormat = GL_RGBA16UI;
				externalFormat = GL_RGBA16UI;
				dataType = GL_UNSIGNED_SHORT;
			} else if (channels == 3) {
				internalFormat = GL_RGB16UI;
				externalFormat = GL_RGB16UI;
				dataType = GL_UNSIGNED_SHORT;
			} else if (channels == 1) {
				internalFormat = GL_R16UI;
				externalFormat = GL_R16UI;
				dataType = GL_UNSIGNED_SHORT;
			} else {
				throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
			}
			break;
		case ImageType::UINT:
			if (channels == 4) {
				internalFormat = GL_RGBA32UI;
				externalFormat = GL_RGBA32UI;
				dataType = GL_UNSIGNED_INT;
			} else if (channels == 3) {
				internalFormat = GL_RGB32UI;
				externalFormat = GL_RGB32UI;
				dataType = GL_UNSIGNED_INT;
			} else if (channels == 1) {
				internalFormat = GL_R32UI;
				externalFormat = GL_R32UI;
				dataType = GL_UNSIGNED_INT;
			} else {
				throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
			}
			break;
		case ImageType::BYTE:
			if (channels == 4) {
				internalFormat = GL_RGBA;
				externalFormat = GL_RGBA;
				dataType = GL_BYTE;
			} else if (channels == 3) {
				internalFormat = GL_RGB;
				externalFormat = GL_RGB;
				dataType = GL_BYTE;
			} else if (channels == 1) {
				internalFormat = GL_R;
				externalFormat = GL_R;
				dataType = GL_BYTE;
			} else {
				throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
			}
			break;
		case ImageType::SHORT:
			if (channels == 4) {
				internalFormat = GL_RGBA16I;
				externalFormat = GL_RGBA16I;
				dataType = GL_SHORT;
			} else if (channels == 3) {
				internalFormat = GL_RGB16I;
				externalFormat = GL_RGB16I;
				dataType = GL_SHORT;
			} else if (channels == 1) {
				internalFormat = GL_R16I;
				externalFormat = GL_R16I;
				dataType = GL_SHORT;
			} else {
				throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
			}
			break;
		case ImageType::INT:
			if (channels == 4) {
				internalFormat = GL_RGBA32I;
				externalFormat = GL_RGBA32I;
				dataType = GL_INT;
			} else if (channels == 3) {
				internalFormat = GL_RGB32I;
				externalFormat = GL_RGB32I;
				dataType = GL_INT;
			} else if (channels == 1) {
				internalFormat = GL_R32I;
				externalFormat = GL_R32I;
				dataType = GL_INT;
			} else {
				throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
			}
			break;
		default:
			throw std::runtime_error(MakeString() << "Texture format not supported " << type << channels);
		}
		glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, externalFormat, dataType, nullptr);
		if (buffer != nullptr)
			clReleaseMemObject(buffer);
		buffer = clCreateFromGLTexture(CLContext(), f, GL_TEXTURE_2D, 0, textureId, &err);
		if (err != CL_SUCCESS) {
			throw ocl_runtime_error(
					aly::MakeString() << "Could not create 2D image [w=" << width << ",h=" << height << ",c=" << channels << ",b=" << typeSize << "].", err);
		}
		regions[0] = width;
		regions[1] = height;
		regions[2] = 1;

		glBindTexture(GL_TEXTURE_2D, 0);

		return textureId;
	}

	void Image2dCLGL::read(void* data, bool block) const {
		const size_t origin[3] = { 0, 0, 0 };
		std::vector<cl_mem> memobjs = { getHandle() };
		CLInstance()->lockGL(memobjs);
		int err = clEnqueueReadImage(CLQueue(), buffer, (block) ? CL_TRUE : CL_FALSE, origin, &regions[0], width * channels * typeSize, 0, data, 0, nullptr,
				nullptr);
		clFlush(CLQueue());
		CLInstance()->unlockGL(memobjs);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not read CL/GL image.", err);
	}

	void Image2dCLGL::write(const void* data, bool block) {
		const size_t origin[3] = { 0, 0, 0 };
		std::vector<cl_mem> memobjs = { getHandle() };
		CLInstance()->lockGL(memobjs);
		int err = clEnqueueWriteImage(CLQueue(), buffer, (block) ? CL_TRUE : CL_FALSE, origin, &regions[0], width * channels * typeSize, 0, data, 0, nullptr,
				nullptr);
		clFlush(CLQueue());
		CLInstance()->unlockGL(memobjs);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not write CL/GL image.", err);
	}
}

