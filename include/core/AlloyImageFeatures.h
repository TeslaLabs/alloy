
#ifndef _IMAGEFEATURES_H
#define _IMAGEFEATURES_H
#include <AlloyImage.h>
#include <array>
namespace aly {
	namespace daisy {
		typedef std::vector<Image1f> Cube1f;
		inline void point_transform_via_homography(float* H, float x, float y, float &u, float &v)
		{
			float kxp = H[0] * x + H[1] * y + H[2];
			float kyp = H[3] * x + H[4] * y + H[5];
			float kp = H[6] * x + H[7] * y + H[8];
			u = kxp / kp;
			v = kyp / kp;
		}

		inline float epipolar_line_slope(float y, float x, float* F)
		{
			float line[3];
			line[0] = F[0] * x + F[1] * y + F[2];
			line[1] = F[3] * x + F[4] * y + F[5];
			line[2] = F[6] * x + F[7] * y + F[8];

			float m = -line[0] / line[1];
			float slope = std::atan(m)*180/ALY_PI;

			if (slope <    0) slope += 360;
			if (slope >= 360) slope -= 360;

			return slope;
		}


		template<class T>
		class rectangle
		{
		public:
			T lx, ux, ly, uy;
			T dx, dy;
			rectangle(T xl, T xu, T yl, T yu) { lx = xl; ux = xu; ly = yl; uy = yu; dx = ux - lx; dy = uy - ly; };
			rectangle() { lx = ux = ly = uy = dx = dy = 0; };
		};

	
		/// checks if the number x is between lx - ux interval.
		/// the equality is checked depending on the value of le and ue parameters.
		/// if le=1 => lx<=x is checked else lx<x is checked
		/// if ue=1 => x<=ux is checked else x<ux is checked
		/// by default x is searched inside of [lx,ux)
		template<class T1, class T2, class T3> inline
			bool is_inside(T1 x, T2 lx, T3 ux, bool le = true, bool ue = false)
		{
			if ((((lx<x) && (!le)) || ((lx <= x) && le)) && (((x<ux) && (!ue)) || ((x <= ux) && ue)))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		/// checks if the number x is between lx - ux and/or y is between ly - uy interval.
		/// If the number is inside, then function returns true, else it returns false.
		/// the equality is checked depending on the value of le and ue parameters.
		/// if le=1 => lx<=x is checked else lx<x is checked
		/// if ue=1 => x<=ux is checked else x<ux is checked
		/// by default x is searched inside of [lx,ux).
		/// the same equality check is applied to the y variable as well.
		/// If the 'oper' is set '&' both of the numbers must be within the interval to return true
		/// But if the 'oper' is set to '|' then only one of them being true is sufficient.
		template<class T1, class T2, class T3> inline
			bool is_inside(T1 x, T2 lx, T3 ux, T1 y, T2 ly, T3 uy, bool le = true, bool ue = false, char oper = '&')
		{
			switch (oper)
			{
			case '|':
				if (is_inside(x, lx, ux, le, ue) || is_inside(y, ly, uy, le, ue))
					return true;
				return false;

			default:
				if (is_inside(x, lx, ux, le, ue) && is_inside(y, ly, uy, le, ue))
					return true;
				return false;
			}
		}
		template<class T1, class T2> inline
			bool is_inside(T1 x, T1 y, rectangle<T2> roi, bool le = true, bool ue = false, char oper = '&')
		{
			switch (oper)
			{
			case '|':
				if (is_inside(x, roi.lx, roi.ux, le, ue) || is_inside(y, roi.ly, roi.uy, le, ue))
					return true;
				return false;

			default:
				if (is_inside(x, roi.lx, roi.ux, le, ue) && is_inside(y, roi.ly, roi.uy, le, ue))
					return true;
				return false;
			}
		}

		/// checks if the number x is outside lx - ux interval
		/// the equality is checked depending on the value of le and ue parameters.
		/// if le=1 => lx>x is checked else lx>=x is checked
		/// if ue=1 => x>ux is checked else x>=ux is checked
		/// by default is x is searched outside of [lx,ux)
		template<class T1, class T2, class T3> inline
			bool is_outside(T1 x, T2 lx, T3 ux, bool le = true, bool ue = false)
		{
			return !(is_inside(x, lx, ux, le, ue));
		}

		/// checks if the numbers x and y is outside their intervals.
		/// The equality is checked depending on the value of le and ue parameters.
		/// If le=1 => lx>x is checked else lx>=x is checked
		/// If ue=1 => x>ux is checked else x>=ux is checked
		/// By default is x is searched outside of [lx,ux) (Similarly for y)
		/// By default, 'oper' is set to OR. If one of them is outside it returns
		/// true otherwise false.
		template<class T1, class T2, class T3> inline
			bool is_outside(T1 x, T2 lx, T3 ux, T1 y, T2 ly, T3 uy, bool le = true, bool ue = false, char oper = '|')
		{
			switch (oper)
			{
			case '&':
				if (is_outside(x, lx, ux, le, ue) && is_outside(y, ly, uy, le, ue))
					return true;
				return false;
			default:
				if (is_outside(x, lx, ux, le, ue) || is_outside(y, ly, uy, le, ue))
					return true;
				return false;
			}
		}

		/// checks if the numbers x and y is outside their intervals.
		/// The equality is checked depending on the value of le and ue parameters.
		/// If le=1 => lx>x is checked else lx>=x is checked
		/// If ue=1 => x>ux is checked else x>=ux is checked
		/// By default is x is searched outside of [lx,ux) (Similarly for y)
		/// By default, 'oper' is set to OR. If one of them is outside it returns
		/// true otherwise false.
		template<class T1, class T2> inline
			bool is_outside(T1 x, T1 y, rectangle<T2> roi, bool le = true, bool ue = false, char oper = '|')
		{
			switch (oper)
			{
			case '&':
				if (is_outside(x, roi.lx, roi.ux, le, ue) && is_outside(y, roi.ly, roi.uy, le, ue))
					return true;
				return false;
			default:
				if (is_outside(x, roi.lx, roi.ux, le, ue) || is_outside(y, roi.ly, roi.uy, le, ue))
					return true;
				return false;
			}
		}
		/// returns the theta component of a point in the range -PI to PI.
		template<class T> inline
			float angle(T x, T y)
		{
			return std::atan2((float)y, (float)x);
		}

		/// returns the theta component of a point array in the range -PI to PI.
		template<class T> inline
			float* angle(T* x, T* y, int lsz)
		{
			float* ang = allocate<float>(lsz);

			for (int k = 0; k<lsz; k++)
			{
				ang[k] = angle<T>(x[k], y[k]);
			}

			return ang;
		}

		/// returns the radial component of a point.
		template<class T> inline
			T magnitude(T x, T y)
		{
			return std::sqrt(x*x + y*y);
		}

		/// computes the radial component of a 2D array and returns the
		/// result in a REAL array. the x&y coordinates are given in
		/// separate 1D arrays together with their size.
		template<class T> inline
			T* magnitude(T* arrx, T* arry, int lsz)
		{
			T* mag = allocate<T>(lsz);

			for (int k = 0; k<lsz; k++)
			{
				mag[k] = sqrt(arrx[k] * arrx[k] + arry[k] * arry[k]);
			}

			return mag;
		}

		/// Converts the given cartesian coordinates of a point to polar
		/// ones.
		template<class T> inline
			void cartesian2polar(T x, T y, float &r, float &th)
		{
			r = magnitude(x, y);
			th = angle(x, y);
		}

		/// Converts the given polar coordinates of a point to cartesian
		/// ones.
		template<class T1, class T2> inline
			void polar2cartesian(T1 r, T1 t, T2 &y, T2 &x)
		{
			x = (T2)(r * std::cos(t));
			y = (T2)(r * std::sin(t));
		}
		enum class Normalization
		{
			Partial = 0, Full = 1, Sift = 2, Default = 3
		};
		class Descriptor: public std::vector<float> {
		public:
			Descriptor() :std::vector<float>() {
			}
		};
		class DescriptorField {
		protected:
			std::vector<Descriptor> data;
		public:
			int width, height;
			DescriptorField(int w = 0, int h = 0):width(w),height(h) {

			}
			size_t size() const {
				return data.size();
			}
			void resize(int w, int h) {
				data.resize(w * h);
				data.shrink_to_fit();
				width = w;
				height = h;
			}
			inline void clear() {
				data.clear();
				data.shrink_to_fit();
				width = 0;
				height = 0;
			}
			Descriptor* ptr() {
				if (data.size() == 0)
					return nullptr;
				return data.data();
			}
			const Descriptor* ptr() const {
				if (data.size() == 0)
					return nullptr;
				return data.data();
			}
			const Descriptor& operator[](const size_t i) const {
				return data[i];
			}
			Descriptor& operator[](const size_t i) {
				return data[i];
			}
			Descriptor& operator()(int i, int j) {
				return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
			}
			Descriptor& operator()(const int2 ij) {
				return data[clamp(ij.x, 0, width - 1)+ clamp(ij.y, 0, height - 1) * width];
			}
			const Descriptor& operator()(int i, int j) const {
				return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
			}
			const Descriptor& operator()(const int2 ij) const {
				return data[clamp(ij.x, 0, width - 1)+ clamp(ij.y, 0, height - 1) * width];
			}
		};
		class Daisy {
		protected:
			static const float sigma_0;
			static const float sigma_1;
			static const float sigma_2;
			static const float sigma_step;
			static const int scale_st;
			static const int scale_en = 1;
			static const int ORIENTATIONS = 360;
			static const int cubeNumber = 3;

			Image1f image;
			Image1f scaleMap;
			Image1i orientMap;
			DescriptorField descriptorField;
			std::vector<float2> gridPoints;
			std::vector<std::vector<Image1f>> smoothLayers;
			std::vector<ImageRGBAf> gradientPyramid;
			bool scaleInvariant;
			bool rotationInvariant;
			int orientationResolutions;
			std::array<int,64> selectedCubes; 
			std::vector<float> sigmas;
			std::array<std::vector<float2>, ORIENTATIONS> orientedGridPoints;
			int numberOfGridPoints;
			int descriptorSize;
			std::array<float,360> orientationShift;
			bool disableInterpolation;
			int cubeSize;
			int layerSize;
			float descriptorRadius;
			int radiusBins;
			int angleBins;
			int histogramBins;
			Normalization normalizationType;
			void computeCubeSigmas();
			void computeGridPoints();
			void computeDescriptors();
			void computeScales();
			void computeHistograms();
			void computeOrientations();
			void computeDescriptor(float i,float j, int orientation, Descriptor& out);
			void computeDescriptor(int i, int j, Descriptor& descriptor);
			void updateSelectedCubes();
			void computeOrientedGridPoints();
			void computeSmoothedGradientLayers();
			float interpolatePeak(float left, float center, float right);
			void smoothHistogram(std::vector<float>& hist, int hsz);
			void layeredGradient(const Image1f& image, std::vector<Image1f>& layers, int layer_no = 8);
			int quantizeRadius(float rad);
			void getUnnormalizedDescriptor(float x, float y, int orientation, Descriptor& descriptor);
			void normalizeSiftWay(Descriptor& desc);
			void normalizePartial(Descriptor& desc);
			void normalizeFull(Descriptor& desc);
			void normalizeDescriptor(Descriptor& desc, Normalization nrm_type);
			void computeHistogram(const std::vector<Image1f>& hcube, int x, int y, std::vector<float>& histogram);
			void i_get_descriptor(float x,float y, int orientation, Descriptor& descriptor);
			void i_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<Image1f>& cube);
			void bi_get_histogram(std::vector<float>& histogram, float x, float y,int shift, const std::vector<Image1f>& cube);
			void ti_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<Image1f>& cube);
			void ni_get_histogram(std::vector<float>& histogram, int x, int y, int shift, const std::vector<Image1f>& hcube);
			void ni_get_descriptor(float x, float y, int orientation, Descriptor& descriptor);
		public:
			Daisy();
			void evaluate(const ImageRGBAf& image, float descriptorRadius=15.0f, int radiusBins=3, int angleBins=8, int histogramBins=8);
		};
	}
}
#endif