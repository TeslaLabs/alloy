
#ifndef _IMAGEFEATURES_H
#define _IMAGEFEATURES_H
#include <AlloyImage.h>
#include <array>
namespace aly {
	namespace daisy {
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

		template<class T1,class T2> inline bool is_outside(T1 x, T2 lx, T2 ux, T1 y, T2 ly, T2 uy)
		{
			return (x<lx || y>ux || y<ly || y>uy);
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

		class Layer {
		protected:
			std::vector<float> data;
		public:
			int width, height;
			Layer(int w = 0, int h = 0) :width(w), height(h) {

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
			float* ptr() {
				if (data.size() == 0)
					return nullptr;
				return data.data();
			}
			const float* ptr() const {
				if (data.size() == 0)
					return nullptr;
				return data.data();
			}
			const float& operator[](const size_t i) const {
				return data[i];
			}
			float& operator[](const size_t i) {
				return data[i];
			}
			float& operator()(int i, int j) {
				return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
			}
			float& operator()(const int2 ij) {
				return data[clamp(ij.x, 0, width - 1)
					+ clamp(ij.y, 0, height - 1) * width];
			}
			const float& operator()(int i, int j) const {
				return data[clamp(i, 0, width - 1) + clamp(j, 0, height - 1) * width];
			}
			const float& operator()(const int2 ij) const {
				return data[clamp(ij.x, 0, width - 1)
					+ clamp(ij.y, 0, height - 1) * width];
			}

			void setZero() {
				data.assign(data.size(), 0.0f);
			}
			float operator()(float x, float y) {
				int i = static_cast<int>(std::floor(x));
				int j = static_cast<int>(std::floor(y));
				float rgb00 = operator()(i, j);
				float rgb10 = operator()(i + 1, j);
				float rgb11 = operator()(i + 1, j + 1);
				float rgb01 = operator()(i, j + 1);
				float dx = x - i;
				float dy = y - j;
				return ((rgb00 * (1.0f - dx) + rgb10 * dx) * (1.0f - dy)
					+ (rgb01 * (1.0f - dx) + rgb11 * dx) * dy);
			}
			const float operator()(float x, float y) const {
				int i = static_cast<int>(std::floor(x));
				int j = static_cast<int>(std::floor(y));
				const float rgb00 = operator()(i, j);
				const float rgb10 = operator()(i + 1, j);
				const float rgb11 = operator()(i + 1, j + 1);
				const float rgb01 = operator()(i, j + 1);
				float dx = x - i;
				float dy = y - j;
				return ((rgb00 * (1.0f - dx) + rgb10 * dx) * (1.0f - dy)
					+ (rgb01 * (1.0f - dx) + rgb11 * dx) * dy);
			}
		};
		typedef std::vector<Layer> LayeredImage;
		void WriteLayeredImageToFile(const std::string& file, const LayeredImage& img);

		template<size_t M, size_t N> void GaussianKernel(float(&kernel)[M][N],
			float sigmaX = float(0.607902736 * (M - 1) * 0.5),
			float sigmaY = float(0.607902736 * (N - 1) * 0.5)) {
			float sum = 0;
			for (int i = 0; i < (int)M; i++) {
				for (int j = 0; j < (int)N; j++) {
					float x = float(i - 0.5 * (M - 1));
					float y = float(j - 0.5 * (N - 1));
					float xn = x / sigmaX;
					float yn = y / sigmaY;
					float w = float(std::exp(-0.5 * (xn * xn + yn * yn)));
					sum += w;
					kernel[i][j] = w;
				}
			}
			sum = 1.0f / sum;
			for (int i = 0; i < (int)M; i++) {
				for (int j = 0; j < (int)N; j++) {
					kernel[i][j] *= sum;
				}
			}
		}

		template<size_t M, size_t N> void Smooth(const Layer& image, Layer& B,float sigmaX = (0.607902736 * (M - 1) * 0.5),float sigmaY = (0.607902736 * (N - 1) * 0.5)) {
			float filter[M][N];
			GaussianKernel<M,N>(filter, sigmaX, sigmaY);
			B.resize(image.width, image.height);
#pragma omp parallel for
			for (int j = 0; j < image.height; j++) {
				for (int i = 0; i < image.width; i++) {
					float vsum=0.0;
					for (int ii = 0; ii < (int)M; ii++) {
						for (int jj = 0; jj < (int)N; jj++) {
							float val = image(i + ii - (int)M / 2, j + jj - (int)N / 2);
							vsum += filter[ii][jj] * val;
						}
					}
					B(i, j) = vsum;
				}
			}
		}
		void Smooth(const Layer& image, Layer& B, float sigmaX, float sigmaY);
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
			static const float sigma_init;
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
			std::vector<LayeredImage> smoothLayers;
			bool scaleInvariant;
			bool rotationInvariant;
			int orientationResolutions;
			std::vector<int> selectedCubes; 
			std::vector<float> sigmas;
			std::array<std::vector<float2>, ORIENTATIONS> orientedGridPoints;
			int numberOfGridPoints;
			int descriptorSize;
			std::array<float,360> orientationShift;
			bool disableInterpolation;
			float descriptorRadius;
			int radiusBins;
			int angleBins;
			int histogramBins;
			Normalization normalizationType;
			void computeCubeSigmas();
			void computeGridPoints();
			void computeScales();
			void computeHistograms();
			void computeOrientations();
			void updateSelectedCubes();
			void computeOrientedGridPoints();
			void computeSmoothedGradientLayers();
			float interpolatePeak(float left, float center, float right);
			void smoothHistogram(std::vector<float>& hist, int hsz);
			void layeredGradient(const Image1f& image, LayeredImage& layers, int layer_no = 8);
			int quantizeRadius(float rad);
			void getUnnormalizedDescriptor(float x, float y, int orientation, Descriptor& descriptor);
			void normalizeSiftWay(Descriptor& desc);
			void normalizePartial(Descriptor& desc);
			void normalizeFull(Descriptor& desc);
			void normalizeDescriptor(Descriptor& desc, Normalization nrm_type);
			void computeHistogram(const LayeredImage& hcube, int x, int y, std::vector<float>& histogram);
			void i_get_descriptor(float x,float y, int orientation, Descriptor& descriptor);
			void i_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<Layer>& cube);
			void bi_get_histogram(std::vector<float>& histogram, float x, float y,int shift, const std::vector<Layer>& cube);
			void ti_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<Layer>& cube);
			void ni_get_histogram(std::vector<float>& histogram, int x, int y, int shift, const std::vector<Layer>& hcube);
			void ni_get_descriptor(float x, float y, int orientation, Descriptor& descriptor);
		public:
			Daisy();
			void initialize();
			void evaluate(const ImageRGBAf& image, float descriptorRadius=15.0f, int radiusBins=3, int angleBins=8, int histogramBins=8);
			void computeDescriptors();
			void computeDescriptor(float i, float j, int orientation, Descriptor& out);
			void computeDescriptor(int i, int j, Descriptor& descriptor);

		};
	}
}
#endif