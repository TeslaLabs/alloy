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

#ifndef INCLUDE_IMAGE2DCLGL
#define INCLUDE_IMAGE2DCLGL
#include "Image2dCL.h"
#include <GLTexture.h>
namespace aly {
	class Image2dCLGL: public Image2dCL {
	protected:
		ImageType imageType;
		void read(void* data, bool block = true) const;
		void write(const void* data, bool block = true);
		int create(cl_mem_flags f, unsigned int glId, const ImageType& type, int w, int h, int c, int bytesPerChannel);
	public:
		Image2dCLGL();
		~Image2dCLGL();
		template<class T> void read(std::vector<T>& data, bool block = true) const {
			read(data.data(), block);
		}
		template<class T> void write(const std::vector<T>& data, bool block = true) {
			write(data.data(), block);
		}
		template<class T, int C, ImageType I> int create(cl_mem_flags f, const GLTexture<T, C, I>& texture) {
			return create(f, texture.getTextureId(), texture.width(), texture.height(), C, sizeof(T));
		}
		template<class T, int C, ImageType I> int create(const GLTexture<T, C, I>& texture) {
			return create(CL_MEM_READ_WRITE, texture.getTextureId(), texture.width(), texture.height(), C, sizeof(T));
		}
		ImageType getImageType() const {
			return imageType;
		}
		int create(cl_mem_flags f, const ImageType& type, int w, int h, int ch, int typeSize);
		template<class T, int C, ImageType I> Image2dCLGL(const GLTexture<T, C, I>& img, const cl_image_format& formats = CL_MEM_READ_WRITE) :
				Image2dCLGL() {
			create(format, img);
		}
		template<class T, int M, ImageType I> void read(Image<T, M, I>& vec, bool block = true) const {
			read(vec.data, block);
		}
		template<class T, int M, ImageType I> void write(const Image<T, M, I>& vec, bool block = true) {
			write(vec.data, block);
		}
	};
}
#endif
