
#ifndef _IMAGEFEATURES_H
#define _IMAGEFEATURES_H
#include <AlloyImage.h>
#include <array>
namespace aly {
	namespace daisy {

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
			Image1f image;
			Image1f scaleMap;
			Image1i orientMap;
			DescriptorField descriptorField;
			std::vector<float2> gridPoints;
			std::vector<ImageRGBAf> gradientPyramid;
			bool scaleInvariant;
			bool rotationInvariant;
			int orientationResolutions;
			std::array<int,64> selectedCubes; 
			std::vector<float> sigmas;
			std::array<std::vector<float2>, ORIENTATIONS> orientedGridPoints;
			int numberOfGridPoints;
			int descriptorSize;
			std::array<float,360> orientatioShift;
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
			void computeOrientations();
			void computeDescriptor(int i,int j, int orientation, Descriptor& out);
			void updateSelectedCubes();
			void computeOrientedGridPoints();
			float interpolatePeak(float left, float center, float right);
			void smoothHistogram(std::vector<float>& hist, int hsz);
			void layeredGradient(const Image1f& image, std::vector<Image1f>& layers, int layer_no = 8);
			int quantizeRadius(float rad);
		public:
			Daisy(int descSize);
			void evaluate(const ImageRGBAf& image, float descriptorRadius, int radiusBins, int angleBins, int histogramBins);
		};
	}
}
#endif