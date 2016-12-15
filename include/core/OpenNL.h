#pragma once

#include "nl/nl.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "AlloyMath.h"
namespace aly
{
	template <class T> inline T nl_min(T x, T y)
	{
		return x < y ? x : y;
	}
	template <class T> inline T nl_max(T x, T y)
	{
		return x > y ? x : y;
	}

	struct Vector2
	{
		Vector2(double x_in, double y_in) : x(x_in), y(y_in) { }
		Vector2() : x(0), y(0) { }
		double x;
		double y;
	};

	struct Vector3
	{
		Vector3(double x_in, double y_in, double z_in) : x(x_in), y(y_in), z(z_in) { }
		Vector3() : x(0), y(0), z(0) { }
		double length() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}
		void normalize()
		{
			double l = length();
			x /= l; y /= l; z /= l;
		}
		double x;
		double y;
		double z;
	};

	// I/O

	std::ostream &operator<<(std::ostream &out, const Vector2 &v);
	std::ostream &operator<<(std::ostream &out, const Vector3 &v);
	std::istream &operator>>(std::istream &in, Vector2 &v);
	std::istream &operator>>(std::istream &in, Vector3 &v);

	double operator*(const Vector3 &v1, const Vector3 &v2); // dot product 
	Vector3 operator^(const Vector3 &v1, const Vector3 &v2); // cross product 
	Vector3 operator+(const Vector3 &v1, const Vector3 &v2);
	Vector3 operator-(const Vector3 &v1, const Vector3 &v2);
	Vector2 operator+(const Vector2 &v1, const Vector2 &v2);
	Vector2 operator-(const Vector2 &v1, const Vector2 &v2);

	/***********************************************************************************/
	/* Mesh class */
	// Note1: this is a minimum mesh class, it does not have facet adjacency information
	// (we do not need it for LSCM). This is just like an Inventor indexed face set.
	//
	// Note2: load() and save() use Alias|Wavefront .obj file format
	struct Vertex
	{
		Vertex() : locked(false), id(-1) { }
		Vertex( const Vector3 &p, const Vector2 &t) : point(p), tex_coord(t), locked(false), id(-1) {}
		Vector3 point;
		Vector2 tex_coord;
		bool locked;
		int id;
	};

	struct Facet : public std::vector<int> {};

	class IndexedMesh
	{
	public:
		IndexedMesh() : in_facet(false) { }

		Vertex *add_vertex()
		{
			vertex.push_back(Vertex());
			vertex.rbegin()->id = static_cast<int>(vertex.size()) - 1;
			return &*(vertex.rbegin());
		}

		Vertex *add_vertex(const Vector3 &p, const Vector2 &t)
		{
			vertex.push_back(Vertex(p, t));
			vertex.rbegin()->id = static_cast<int>(vertex.size()) - 1;
			return &*(vertex.rbegin());
		}

		void begin_facet()
		{
			assert(!in_facet);
			facet.push_back(Facet());
			in_facet = true;
		}

		void end_facet()
		{
			assert(in_facet);
			in_facet = false;
		}

		void add_vertex_to_facet(unsigned int i)
		{
			assert(in_facet);
			assert(i < vertex.size());
			facet.rbegin()->push_back(i);
		}

		void clear()
		{
			vertex.clear();
			facet.clear();
		}

		void load(const std::string &file_name)
		{
			std::ifstream input(file_name.c_str());
			clear();
			int vti = 0;
			while (input)
			{
				char line[1024];
				input.getline(line, 1024);
				std::stringstream line_input(line);
				std::string keyword;
				line_input >> keyword;
				if (keyword == "v")
				{
					Vector3 p;
					line_input >> p;
					add_vertex(p, Vector2(0, 0));
				}
				else if (keyword == "vt")
				{
					// Ignore tex vertices
					Vector2 p;
					line_input >> p;
					vertex[vti].tex_coord = p;
					vti++;
				}
				else if (keyword == "f")
				{
					begin_facet();
					while (line_input)
					{
						std::string s;
						line_input >> s;
						if (s.length() > 0)
						{
							std::stringstream v_input(s.c_str());
							int index;
							v_input >> index;
							add_vertex_to_facet(index - 1);
							char c;
							v_input >> c;
							if (c == '/')
							{
								v_input >> index;
								// Ignore tex vertex index
							}
						}
					}
					end_facet();
				}
			}

			std::cout << "Loaded " << vertex.size() << " vertices and " << facet.size() << " facets" << std::endl;

		}

		void save(const std::string &file_name)
		{
			unsigned int i, j;
			std::cout << "Write " << file_name << "... ";
			std::ofstream out(file_name.c_str());
			for (i = 0; i < vertex.size(); i++)
			{
				out << "v " << vertex[i].point << std::endl;
			}
			for (i = 0; i < vertex.size(); i++)
			{
				out << "vt " << vertex[i].tex_coord << std::endl;
			}
			for (i = 0; i < facet.size(); i++)
			{
				out << "f ";
				const Facet &F = facet[i];
				for (j = 0; j < F.size(); j++)
				{
					out << (F[j] + 1) << "/" << (F[j] + 1) << " ";
				}
				out << std::endl;
			}
			for (i = 0; i < vertex.size(); i++)
			{
				if (vertex[i].locked)
				{
					out << "# anchor " << i + 1 << std::endl;
				}
			}
			std::cout << "Done." << std::endl;
		}

		std::vector<Vertex> vertex;
		std::vector<Facet>  facet;
		bool in_facet;

	};
	class LSCM {
	public:

		static LSCM* currentLCSM;
		LSCM(IndexedMesh& m) : mesh_(&m) {
		}

		static void SolverMonitor(float progress);
		void apply(const char* type_solver, double errorTolerance, int maxIterations);
	protected:

		void setup_lscm();
		// Note: no-need to triangulate the facet,
		// we can do that "virtually", by creating triangles
		// radiating around vertex 0 of the facet.
		// (however, this may be invalid for concave facets)
		void setup_lscm(const Facet& F);
		// Computes the coordinates of the vertices of a triangle
		// in a local 2D orthonormal basis of the triangle's plane.
		static void project_triangle(
			const Vector3& p0,
			const Vector3& p1,
			const Vector3& p2,
			Vector2& z0,
			Vector2& z1,
			Vector2& z2
			);

		// LSCM equation, geometric form :
		// (Z1 - Z0)(U2 - U0) = (Z2 - Z0)(U1 - U0)
		// Where Uk = uk + i.vk is the complex number 
		//                       corresponding to (u,v) coords
		//       Zk = xk + i.yk is the complex number 
		//                       corresponding to local (x,y) coords
		// cool: no divide with this expression,
		//  makes it more numerically stable in
		//  the presence of degenerate triangles.

		void setup_conformal_map_relations(
			const Vertex& v0, const Vertex& v1, const Vertex& v2
			);

		/**
		* copies u,v coordinates from OpenNL solver to the mesh.
		*/
		void solver_to_mesh();

		/**
		* copies u,v coordinates from the mesh to OpenNL solver.
		*/
		void mesh_to_solver();


		// Chooses an initial solution, and locks two vertices
		void project();

		IndexedMesh* mesh_;
	};
int OpenNL_Parameterization(std::string solver, IndexedMesh &mesh, double errorTolerance, int maxIterations);

} // end namespace TextureParameterization