
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

		enum class Normalization
		{
			Partial = 0, Full = 1, Sift = 2
		};
		class Descriptor: public std::vector<float> {
		public:
			Descriptor(size_t sz=0) :std::vector<float>(sz) {
			}
			Descriptor(const std::vector<float>& data):std::vector<float>(data.size()) {
				std::vector<float>::assign(data.begin(), data.end());
			}
		};

		class OrientationLayer  {
		protected:
			std::vector<float> data;
		public:
			int width, height;
			OrientationLayer (int w = 0, int h = 0) :width(w), height(h) {

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
			float operator()(float x, float y) const {
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
		typedef std::vector<OrientationLayer > LayeredImage;
		void WriteLayeredImageToFile(const std::string& file, const LayeredImage& img);



		inline double dot(const Descriptor& a,const Descriptor& b) {
			double ret = 0.0;
			int N = (int)std::min(a.size(), b.size());
			for (int i = 0; i < N; i++) {
				ret += a[i] * b[i];
			}
			return ret;
		}
		inline double lengthL2(const Descriptor& a);
		inline double lengthSqr(const Descriptor& a);
		inline double lengthL1(const Descriptor& a);
		inline double angle(const Descriptor& a, const Descriptor& b);
		void Smooth(const OrientationLayer & image, OrientationLayer & B, float sigma);
		class DescriptorField {
		protected:
			std::vector<Descriptor> data;
		public:
			int width, height;
			DescriptorField(int w = 0, int h = 0):width(w),height(h),data(w*h) {

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
			void get(float x, float y, Descriptor& out) const {
				int i = static_cast<int>(std::floor(x));
				int j = static_cast<int>(std::floor(y));
				const Descriptor& rgb00 = operator()(i, j);
				const Descriptor& rgb10 = operator()(i + 1, j);
				const Descriptor& rgb11 = operator()(i + 1, j + 1);
				const Descriptor& rgb01 = operator()(i, j + 1);
				float dx = x - i;
				float dy = y - j;
				int N = (int)rgb00.size();
				out.resize(N);
				for (int i = 0; i < N; i++) {
					out[i] = ((rgb00[i] * (1.0f - dx) + rgb10[i] * dx) * (1.0f - dy) + (rgb01[i] * (1.0f - dx) + rgb11[i] * dx) * dy);
				}
			}
			Descriptor operator()(float x, float y) const {
				int i = static_cast<int>(std::floor(x));
				int j = static_cast<int>(std::floor(y));
				const Descriptor& rgb00 = operator()(i, j);
				const Descriptor& rgb10 = operator()(i + 1, j);
				const Descriptor& rgb11 = operator()(i + 1, j + 1);
				const Descriptor& rgb01 = operator()(i, j + 1);
				float dx = x - i;
				float dy = y - j;
				int N = (int)rgb00.size();
				Descriptor out(N);
				for (int i = 0; i < N; i++) {
					out[i]= ((rgb00[i] * (1.0f - dx) + rgb10[i] * dx) * (1.0f - dy)+ (rgb01[i] * (1.0f - dx) + rgb11[i] * dx) * dy);
				}
				return out;
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
			static const int scale_en = 3;
			static const int ORIENTATIONS = 360;
			static const int cubeNumber = 3;
			Image1f image;
			std::vector<float2> gridPoints;
			std::vector<LayeredImage> smoothLayers;
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
			void computeCubeSigmas();
			void computeGridPoints();
			void computeScales(Image1f& scaleMap);
			void computeHistograms();
			void computeOrientations(Image1i& orientMap, const Image1f& scaleMap, bool scaleInvariant);
			void updateSelectedCubes();
			void computeOrientedGridPoints();
			void computeSmoothedGradientLayers();
			float interpolatePeak(float left, float center, float right);
			void smoothHistogram(std::vector<float>& hist, int hsz);
			void layeredGradient(const Image1f& image, LayeredImage& layers, int layer_no = 8);
			int quantizeRadius(float rad);
			void getUnnormalizedDescriptor(float x, float y, int orientation, Descriptor& descriptor,bool disableInterpolation);
			void normalizeSiftWay(Descriptor& desc);
			void normalizePartial(Descriptor& desc);
			void normalizeFull(Descriptor& desc);
			void normalizeDescriptor(Descriptor& desc, Normalization nrm_type);
			void computeHistogram(const LayeredImage& hcube, int x, int y, std::vector<float>& histogram);
			void i_get_descriptor(float x,float y, int orientation, Descriptor& descriptor);
			void i_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<OrientationLayer >& cube);
			void bi_get_histogram(std::vector<float>& histogram, float x, float y,int shift, const std::vector<OrientationLayer >& cube);
			void ti_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<OrientationLayer >& cube);
			void ni_get_histogram(std::vector<float>& histogram, int x, int y, int shift, const std::vector<OrientationLayer >& hcube);
			void ni_get_descriptor(float x, float y, int orientation, Descriptor& descriptor);
			void initialize();

		public:
			Daisy(int orientationResolutions=8);
			void initialize(const ImageRGBAf& image, float descriptorRadius=15.0f, int radiusBins=3, int angleBins=8, int histogramBins=8);
			void getDescriptors(DescriptorField& descriptorField, Normalization  normalizationType = Normalization::Sift, bool scaleInvariant=false, bool rotationInartiant=true, bool disableInterpolation=false);
			void getDescriptor(float i, float j,Descriptor& out, int orientation=0, Normalization normalizationType=Normalization::Sift,bool disableInterpolation = false);
		};
	}
}
#endif