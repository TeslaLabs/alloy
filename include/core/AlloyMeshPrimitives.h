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
#ifndef ALLOYMESHPRIMITIVES_H_
#define ALLOYMESHPRIMITIVES_H_
#include "AlloyMesh.h"
#include "AlloyCamera.h"
namespace aly {
	class Box: public Mesh {
	public:
		Box(const box3f& box,std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Icosahedron : public Mesh {
	public:
		Icosahedron(float radius,std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Sphere : public Mesh {
	public:
		Sphere(float r, int slices, int stacks, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Cylinder : public Mesh {
	public:
		Cylinder(float r, float h, int slices,  std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Torus : public Mesh {
	public:
		Torus(float innerRadius,float outerRadius,int stacks, int slices, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Cone : public Mesh {
	public:
		Cone(float r, float h, int slices, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Pyramid : public Mesh {
	public:
		Pyramid(float w, float h, float d, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Frustum : public Mesh {
	public:
		Frustum(const CameraParameters& cam, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Plane : public Mesh {
	public:
		Plane(float w,float h, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Capsule : public Mesh {
	public:
		Capsule(float r, float h,int slices, int stacks, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class TessellatedSphere : public Mesh {
	public:
		TessellatedSphere(float r,int subdivisions,const SubDivisionScheme& scheme, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Grid : public Mesh {
	public:
		Grid(float w, float h, int rows,int cols, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
	class Asteroid : public Icosahedron {
	public:
		Asteroid(int subdivisions, std::shared_ptr<AlloyContext>& context = AlloyDefaultContext());
	};
}
#endif /* MESH_H_ */
