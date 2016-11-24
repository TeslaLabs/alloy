

#include "AlloyImageFeatures.h"
#include "AlloyImageProcessing.h"

namespace aly {
	namespace daisy {
		const float Daisy::sigma_0 = 1.0f;
		const float Daisy::sigma_1 = std::sqrt(2.0f);
		const float Daisy::sigma_2 = 8.0f;
		const float Daisy::sigma_init = 1.6f;
		const float Daisy::sigma_step = (float)std::pow(2, 1.0f / 2);
		const int Daisy::scale_st = int((std::log(sigma_1 / sigma_0)) / (float)std::log(sigma_step));

		template<class T1, class T2> inline bool is_outside(T1 x, T2 lx, T2 ux, T1 y, T2 ly, T2 uy)
		{
			return (x<lx || y>ux || y<ly || y>uy);
		}
		/// returns the theta component of a point array in the range -PI to PI.
		template<class T> inline
			float* angle(T* x, T* y, int lsz)
		{
			float* ang = allocate<float>(lsz);

			for (int k = 0; k < lsz; k++)
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

		void WriteLayeredImageToFile(const std::string& file, const LayeredImage& img) {
			std::ostringstream vstr;
			std::string fileName = GetFileWithoutExtension(file);
			vstr << fileName << ".raw";
			FILE* f = fopen(vstr.str().c_str(), "wb");
			if (f == NULL) {
				throw std::runtime_error(
					MakeString() << "Could not open " << vstr.str().c_str()
					<< " for writing.");
			}
			int w = 0, h = 0;
			if (img.size() > 0) {
				w = img[0].width;
				h = img[0].height;
			}
			for (int k = 0; k < img.size(); k++) {
				for (int j = 0; j < h; j++) {
					for (int i = 0; i < w; i++) {
						float val = img[k](i, j);
						fwrite(&val, sizeof(float), 1, f);
					}
				}
			}
			fclose(f);
			std::string typeName = "Float";
			std::stringstream sstr;
			sstr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			sstr << "<!-- MIPAV header file -->\n";
			sstr << "<image xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" nDimensions=\"3\">\n";
			sstr << "	<Dataset-attributes>\n";
			sstr << "		<Image-offset>0</Image-offset>\n";
			sstr << "		<Data-type>" << typeName << "</Data-type>\n";
			sstr << "		<Endianess>Little</Endianess>\n";
			sstr << "		<Extents>" << img[0].width << "</Extents>\n";
			sstr << "		<Extents>" << img[0].height << "</Extents>\n";
			sstr << "		<Extents>" << img.size() << "</Extents>\n";
			sstr << "		<Resolutions>\n";
			sstr << "			<Resolution>1.0</Resolution>\n";
			sstr << "			<Resolution>1.0</Resolution>\n";
			sstr << "			<Resolution>1.0</Resolution>\n";
			sstr << "		</Resolutions>\n";
			sstr << "		<Slice-spacing>1.0</Slice-spacing>\n";
			sstr << "		<Slice-thickness>0.0</Slice-thickness>\n";
			sstr << "		<Units>Millimeters</Units>\n";
			sstr << "		<Units>Millimeters</Units>\n";
			sstr << "		<Units>Millimeters</Units>\n";
			sstr << "		<Compression>none</Compression>\n";
			sstr << "		<Orientation>Unknown</Orientation>\n";
			sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
			sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
			sstr << "		<Subject-axis-orientation>Unknown</Subject-axis-orientation>\n";
			sstr << "		<Origin>0.0</Origin>\n";
			sstr << "		<Origin>0.0</Origin>\n";
			sstr << "		<Origin>0.0</Origin>\n";
			sstr << "		<Modality>Unknown Modality</Modality>\n";
			sstr << "	</Dataset-attributes>\n";
			sstr << "</image>\n";
			std::ofstream myfile;
			std::stringstream xmlFile;
			xmlFile << fileName << ".xml";
			myfile.open(xmlFile.str().c_str(), std::ios_base::out);
			if (!myfile.is_open()) {
				throw std::runtime_error(
					MakeString() << "Could not open " << xmlFile.str()
					<< " for writing.");
			}
			myfile << sstr.str();
			myfile.close();
		}
		void gaussian_1d(std::vector<float>& fltr, int fsz, float sigma, float mean)
		{
			int sz = (fsz - 1) / 2;
			int counter = -1;
			float sum = 0.0;
			float v = 2 * sigma*sigma;
			for (int x = -sz; x <= sz; x++)
			{
				counter++;
				fltr[counter] = std::exp((-(x - mean)*(x - mean)) / v);
				sum += fltr[counter];
			}

			if (sum != 0)
			{
				for (int x = 0; x < fsz; x++)
					fltr[x] /= sum;
			}
		}
		template<class T1, class T2> inline
			void conv_buffer_(T1* buffer, T2* kernel, int rsize, int ksize)
		{
			for (int i = 0; i < rsize; i++)
			{
				float sum = 0;
				for (int j = 0; j < ksize; j++)
				{
					sum += buffer[i + j] * kernel[j];
				}
				buffer[i] = sum;
			}
		}
		template<class T1, class T2> inline
			void conv_horizontal(T1* image, int h, int w, T2 *kernel, int ksize)
		{
			int halfsize = ksize / 2;
			assert(w + ksize < 4096);

			T1  buffer[4096];
			for (int r = 0; r < h; r++)
			{
				int rw = r*w;

				for (int i = 0; i < halfsize; i++)
					buffer[i] = image[rw];

				memcpy(&(buffer[halfsize]), &(image[rw]), w * sizeof(T1));

				T1 temp = image[rw + w - 1];
				for (int i = 0; i < halfsize; i++)
					buffer[i + halfsize + w] = temp;

				conv_buffer_(buffer, kernel, w, ksize);

				for (int c = 0; c < w; c++)
					image[rw + c] = buffer[c];
			}
		}

		template<class T1, class T2> inline
			void conv_vertical(T1* image, int h, int w, T2 *kernel, int ksize)
		{
			T1  buffer[4096];

			int halfsize = ksize / 2;
			assert(h + ksize < 4096);

			int h_1w = (h - 1)*w;

			for (int c = 0; c < w; c++)
			{
				for (int i = 0; i < halfsize; i++)
					buffer[i] = image[c];

				for (int i = 0; i < h; i++)
					buffer[halfsize + i] = image[i*w + c];

				for (int i = 0; i < halfsize; i++)
					buffer[halfsize + i + h] = image[h_1w + c];

				conv_buffer_(buffer, kernel, h, ksize);

				for (int r = 0; r < h; r++)
				{
					image[r*w + c] = buffer[r];
				}
			}
		}
		template<typename T> inline
			void convolve_sym_(T* image, int h, int w, T* kernel, int ksize)
		{
			conv_horizontal(image, h, w, kernel, ksize);
			conv_vertical(image, h, w, kernel, ksize);
		}
		void convolve_sym(float* image, int h, int w, float* kernel, int ksize, float* out = NULL)
		{
			if (out == NULL) out = image;
			else memcpy(out, image, sizeof(float)*h*w);
			if (h == 240 && w == 320) { convolve_sym_(out, 240, 320, kernel, ksize); return; }
			if (h == 480 && w == 640) { convolve_sym_(out, 480, 640, kernel, ksize); return; }
			if (h == 512 && w == 512) { convolve_sym_(out, 512, 512, kernel, ksize); return; }
			if (h == 512 && w == 768) { convolve_sym_(out, 512, 768, kernel, ksize); return; }
			if (h == 600 && w == 800) { convolve_sym_(out, 600, 800, kernel, ksize); return; }
			if (h == 768 && w == 1024) { convolve_sym_(out, 768, 1024, kernel, ksize); return; }
			if (h == 1024 && w == 768) { convolve_sym_(out, 1024, 768, kernel, ksize); return; }
			if (h == 256 && w == 256) { convolve_sym_(out, 256, 256, kernel, ksize); return; }
			if (h == 128 && w == 128) { convolve_sym_(out, 128, 128, kernel, ksize); return; }
			if (h == 128 && w == 192) { convolve_sym_(out, 128, 192, kernel, ksize); return; }
			convolve_sym_(out, h, w, kernel, ksize);
		}
		void Smooth(const OrientationLayer & image, OrientationLayer & B, float sigma) {
			int fsz = (int)(5 * sigma);
			if (fsz % 2 == 0) fsz++;
			if (fsz < 3) fsz = 3;
			std::vector<float> filter(fsz);
			gaussian_1d(filter, fsz, sigma, 0);
			B = image;
			convolve_sym(B.ptr(), image.height, image.width, filter.data(), fsz);
		}
		void Smooth(OrientationLayer & image,float sigma) {
			int fsz = (int)(5 * sigma);
			if (fsz % 2 == 0) fsz++;
			if (fsz < 3) fsz = 3;
			std::vector<float> filter(fsz);
			gaussian_1d(filter, fsz, sigma, 0);
			convolve_sym(image.ptr(), image.height, image.width, filter.data(), fsz);
		}
		void Smooth(const Image1f & image, Image1f & B, float sigma) {
			int fsz = (int)(5 * sigma);
			if (fsz % 2 == 0) fsz++;
			if (fsz < 3) fsz = 3;
			std::vector<float> filter(fsz);
			gaussian_1d(filter, fsz, sigma, 0);
			B = image;
			convolve_sym(B.ptr(), image.height, image.width, filter.data(), fsz);
		}

		void Smooth(Image1f & image, float sigma) {
			int fsz = (int)(5 * sigma);
			if (fsz % 2 == 0) fsz++;
			if (fsz < 3) fsz = 3;
			std::vector<float> filter(fsz);
			gaussian_1d(filter, fsz, sigma, 0);
			convolve_sym(image.ptr(), image.height, image.width, filter.data(), fsz);
		}
		Daisy::Daisy(int orientResolutions) :orientationResolutions(orientResolutions) {
		}


		void Daisy::i_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<OrientationLayer >& cube)
		{
			int ishift = (int)shift;
			double fshift = shift - ishift;
			if (fshift < 0.01)
				bi_get_histogram(histogram, x, y, ishift, cube);
			else if (fshift > 0.99)
				bi_get_histogram(histogram, x, y, ishift + 1, cube);
			else
				ti_get_histogram(histogram, x, y, shift, cube);
		}
		void Daisy::bi_get_histogram(std::vector<float>& histogram, float x, float y, int shift, const std::vector<OrientationLayer >& hcube)
		{
			int mnx = int(x);
			int mny = int(y);
			histogram.resize(histogramBins);
			if (mnx >= image.width - 2 || mny >= image.height - 2) {
				histogram.assign(histogram.size(), 0.0f);
				return;
			}
			for (int h = 0; h < histogramBins; h++) {
				if (h + shift < histogramBins) {
					histogram[h] = hcube[h + shift](x, y);
				}
				else {
					histogram[h] = hcube[h + shift - histogramBins](x, y);
				}
			}
		}
		void Daisy::ti_get_histogram(std::vector<float>& histogram, float x, float y, float shift, const std::vector<OrientationLayer >& hcube)
		{
			int ishift = int(shift);
			float layer_alpha = shift - ishift;
			std::vector<float> thist(histogramBins, 0.0f);
			bi_get_histogram(thist, x, y, ishift, hcube);
			histogram.resize(histogramBins);
			for (int h = 0; h < histogramBins - 1; h++) {
				histogram[h] = (1 - layer_alpha) * thist[h] + layer_alpha * thist[h + 1];
			}
			histogram[histogramBins - 1] = (1 - layer_alpha)* thist[histogramBins - 1] + layer_alpha * thist[0];
		}
		void Daisy::ni_get_histogram(std::vector<float>& histogram, int x, int y, int shift, const std::vector<OrientationLayer >& hcube) {
			if (is_outside(x, 0, image.width - 1, y, 0, image.height - 1))
				return;
			histogram.resize(histogramBins);
			for (int h = 0; h < histogramBins; h++)
			{
				int hi = h + shift;
				if (hi >= histogramBins)
					hi -= histogramBins;
				histogram[h] = hcube[hi](x, y);
			}
		}
		void Daisy::i_get_descriptor(float x, float y, int orientation, Descriptor& descriptor) {
			float shift = orientationShift[orientation];
			i_get_histogram(descriptor, x, y, shift, smoothLayers[selectedCubes[0]]);
			descriptor.resize(descriptorSize);
			int r, rdt, region;
			float xx, yy;
			std::vector<float> tmp(histogramBins);
			std::vector<float2>& grid = orientedGridPoints[orientation];
			for (r = 0; r < radiusBins; r++)
			{
				rdt = r * angleBins + 1;
				for (region = rdt; region < rdt + angleBins; region++)
				{
					xx = x + grid[region].x;
					yy = y + grid[region].y;
					if (is_outside(xx, 0, image.width - 1, yy, 0, image.height - 1))
						continue;
					i_get_histogram(tmp, xx, yy, shift, smoothLayers[selectedCubes[r]]);
					for (int i = 0; i < (int)tmp.size(); i++) {
						descriptor[region*histogramBins + i] = tmp[i];
					}
				}
			}
		}
		void Daisy::ni_get_descriptor(float x, float y, int orientation, Descriptor& descriptor) {
			float shift = orientationShift[orientation];
			int ishift = (int)shift;
			if (shift - ishift > 0.5)ishift++;
			int iy = (int)y;
			if (y - iy > 0.5)iy++;
			int ix = (int)x;
			if (x - ix > 0.5)ix++;
			// center
			ni_get_histogram(descriptor, ix, iy, ishift, smoothLayers[selectedCubes[0]]);
			descriptor.resize(descriptorSize);
			float xx, yy;
			float* histogram = 0;
			// petals of the flower
			int r, rdt, region;
			std::vector<float> tmp(histogramBins);
			std::vector<float2>& grid = orientedGridPoints[orientation];
			for (r = 0; r < radiusBins; r++) {
				rdt = r * angleBins + 1;
				for (region = rdt; region < rdt + angleBins; region++) {
					xx = x + grid[region].x;
					yy = y + grid[region].y;
					iy = (int)yy;
					if (yy - iy > 0.5)
						iy++;
					ix = (int)xx;
					if (xx - ix > 0.5)
						ix++;
					if (is_outside(ix, 0, image.width - 1, iy, 0, image.height - 1)) {
						continue;
					}
					ni_get_histogram(tmp, ix, iy, ishift, smoothLayers[selectedCubes[r]]);
					for (int i = 0; i < (int)tmp.size(); i++) {
						descriptor[region*histogramBins + i] = tmp[i];
					}
				}
			}
		}


		void Daisy::initialize(const ImageRGBAf& _image, float _descriptorRadius, int _radiusBins, int _angleBins, int _histogramBins) {
			ConvertImage(_image, image);
			histogramBins = _histogramBins;
			angleBins = _angleBins;
			radiusBins = _radiusBins;
			descriptorRadius = _descriptorRadius;
			numberOfGridPoints = angleBins * radiusBins + 1; // +1 is for center pixel
			descriptorSize = numberOfGridPoints * _histogramBins;
			for (int i = 0; i < ORIENTATIONS; i++) {
				orientationShift[i] = (i / float(ORIENTATIONS)) * _histogramBins;
			}
			computeCubeSigmas();
			computeGridPoints();
			initialize();
		}
		int Daisy::quantizeRadius(float rad) {
			if (rad <= sigmas[0]) return 0;
			if (rad >= sigmas[sigmas.size() - 1]) return (int)(sigmas.size() - 1);
			float dist;
			float mindist = std::numeric_limits<float>::max();
			int mini = 0;
			for (int c = 0; c < sigmas.size(); c++) {
				dist = std::abs(sigmas[c] - rad);
				if (dist < mindist) {
					mindist = dist;
					mini = c;
				}
			}
			return mini;
		}
		void Daisy::computeScales(Image1f& scaleMap) {
			float sigma = std::pow(sigma_step, scale_st)*sigma_0;
			Image1f sim;
			Image1f next_sim;
			Image1f max_dog(image.width, image.height);
			scaleMap.resize(image.width, image.height);
			scaleMap.setZero();
			max_dog.setZero();
			Smooth(image, sim, sigma);
			float sigma_prev = sigma_0;
			float sigma_new;
			float sigma_inc;
			for (int i = 0; i < scale_en; i++) {
				sigma_new = std::pow(sigma_step, scale_st + i) * sigma_0;
				sigma_inc = std::sqrt(sigma_new*sigma_new - sigma_prev*sigma_prev);
				sigma_prev = sigma_new;
				Smooth(sim, next_sim, sigma_inc);
				for (int p = 0; p < (int)image.size(); p++) {
					float dog = std::abs(next_sim[p].x - sim[p].x);
					if (dog > max_dog[p])
					{
						max_dog[p] = float1(dog);
						scaleMap[p] = float1((float)i);
					}
				}
				sim = next_sim;
			}
			//smooth scaling map
			Smooth(scaleMap, sim, 10.0f);
			scaleMap = sim;
			for (int q = 0; q < (int)sim.size(); q++)
			{
				scaleMap[q] = std::floor(scaleMap[q] + 0.5f);
			}
		}

		void Daisy::layeredGradient(const Image1f& image, LayeredImage& layers, int layer_no)
		{
			OrientationLayer  bdata;
			Image1f dx, dy;
			Gradient<3, 3>(image, dx, dy);
			layers.resize(layer_no);
			for (int l = 0; l < layer_no; l++) {
				OrientationLayer & layer_l = layers[l];
				layer_l.resize(image.width, image.height);
				layer_l.setZero();
				float angle = 2.0f * l * ALY_PI / layer_no;
				float cosa = std::cos(angle);
				float sina = std::sin(angle);
				for (int index = 0; index < (int)image.size(); index++) {
					float value = cosa * dx[index] + sina * dy[index];
					if (value > 0) {
						layer_l[index] = float1(value);
					}
					else {
						layer_l[index] = float1(0.0f);
					}
				}
			}
		}
		void Daisy::smoothHistogram(std::vector<float>& hist, int hsz) {
			int i;
			float prev, temp;
			prev = hist[hsz - 1];
			for (i = 0; i < hsz; i++)
			{
				temp = hist[i];
				hist[i] = (prev + hist[i] + hist[(i + 1 == hsz) ? 0 : i + 1]) / 3.0f;
				prev = temp;
			}
		}
		float Daisy::interpolatePeak(float left, float center, float right) {
			if (center < 0.0f)
			{
				left = -left;
				center = -center;
				right = -right;
			}
			float den = (left - 2.0f * center + right);
			if (den == 0) {
				return 0;
			}
			else {
				return 0.5f*(left - right) / den;
			}
		}

		void Daisy::computeHistogram(const LayeredImage& hcube, int x, int y, std::vector<float>& histogram)
		{
			histogram.resize(histogramBins);
			for (int h = 0; h < histogramBins; h++) {
				histogram[h] = hcube[h](x, y);
			}
		}
		void Daisy::computeHistograms()
		{
			std::vector<float> hist;
			for (int r = 0; r < cubeNumber; r++) {
				LayeredImage& dst = smoothLayers[r];
				LayeredImage& src = smoothLayers[r + 1];
				for (int y = 0; y < image.height; y++) {
					for (int x = 0; x < image.width; x++) {
						computeHistogram(src, x, y, hist);
						for (int i = 0; i < histogramBins; i++) {
							dst[i](x, y) = hist[i];
						}
					}
				}
			}
		}
		void Daisy::computeSmoothedGradientLayers() {
			OrientationLayer  tmp;
			float sigma;
			smoothLayers.resize(cubeNumber + 1);
			for (int r = 0; r < smoothLayers.size(); r++) {
				smoothLayers[r].resize(histogramBins);
				for (OrientationLayer & img : smoothLayers[r]) {
					img.resize(image.width, image.height);
				}
			}
			for (int r = 0; r < cubeNumber; r++) {
				LayeredImage& prev_cube = smoothLayers[r];
				LayeredImage& cube = smoothLayers[r + 1];
				// incremental smoothing
				if (r == 0) {
					sigma = sigmas[0];
				}
				else {
					sigma = std::sqrt(sigmas[r] * sigmas[r] - sigmas[r - 1] * sigmas[r - 1]);
				}
				for (int th = 0; th < histogramBins; th++) {
					Smooth(prev_cube[th], cube[th], sigma);
				}
			}
			computeHistograms();
		}
		void Daisy::computeOrientations(Image1i& orientMap, const Image1f& scaleMap, bool scaleInvariant) {
			OrientationLayer  tmp;
			std::vector<OrientationLayer > layers;
			layeredGradient(image, layers, orientationResolutions);
			orientMap.resize(image.width, image.height);
			orientMap.setZero();
			int max_ind;
			float max_val;
			int next, prev;
			float peak, angle;
			float sigma_inc;
			float sigma_prev = 0;
			float sigma_new;
			std::vector<float> hist(orientationResolutions);
			for (int scale = 0; scale < scale_en; scale++) {
				sigma_new = std::pow(sigma_step, scale) * descriptorRadius / 3.0f;
				sigma_inc = std::sqrt(sigma_new*sigma_new - sigma_prev*sigma_prev);
				sigma_prev = sigma_new;
				for (OrientationLayer & layer : layers) {
					Smooth(layer, tmp, sigma_inc);
					layer = tmp;
				}
				for (int j = 0; j < image.height; j++) {
					for (int i = 0; i < image.width; i++) {
						if (scaleInvariant &&scaleMap.size() > 0 && scaleMap(i, j).x != scale) continue;
						for (int ori = 0; ori < orientationResolutions; ori++) {
							hist[ori] = layers[ori](i, j);
						}
						for (int kk = 0; kk < 6; kk++) {
							smoothHistogram(hist, orientationResolutions);
						}
						max_val = -1;
						max_ind = 0;
						for (int ori = 0; ori < orientationResolutions; ori++) {
							if (hist[ori] > max_val) {
								max_val = hist[ori];
								max_ind = ori;
							}
						}
						prev = max_ind - 1;
						if (prev < 0)
							prev += orientationResolutions;
						next = max_ind + 1;
						if (next >= orientationResolutions)
							next -= orientationResolutions;
						peak = interpolatePeak(hist[prev], hist[max_ind], hist[next]);
						angle = (max_ind + peak)*360.0f / orientationResolutions;
						int iangle = (int)std::floor(angle);
						if (iangle < 0) iangle += 360;
						if (iangle >= 360) iangle -= 360;
						if (!(iangle >= 0.0f && iangle < 360.0f)) {
							angle = 0;
						}
						orientMap(i, j) = int1(iangle);
					}
				}
			}
			computeOrientedGridPoints();
		}
		void Daisy::initialize() {
			smoothLayers.resize(1);
			layeredGradient(image, smoothLayers[0], orientationResolutions);
			OrientationLayer  tmp;
			float sigma = std::sqrt(sigma_init*sigma_init - 0.25f);
			// assuming a 0.5 image smoothness, we pull this to 1.6 as in sift
			for (int i = 0; i < smoothLayers[0].size(); i++)
			{
				Smooth(smoothLayers[0][i], tmp, sigma);
				smoothLayers[0][i] = tmp;
			}
			computeSmoothedGradientLayers();
			//for (int i = 0; i < smoothLayers.size(); i++) {
			//	WriteLayeredImageToFile(MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR << "gradient" << i << ".xml", smoothLayers[i]);
			//}
		}
		void Daisy::normalizeDescriptor(Descriptor& desc, Normalization nrm_type)
		{
			if (nrm_type == Normalization::Partial)
				normalizePartial(desc);
			else if (nrm_type == Normalization::Full)
				normalizeFull(desc);
			else if (nrm_type == Normalization::Sift)
				normalizeSiftWay(desc);
		}

		void Daisy::normalizePartial(Descriptor& desc) {
			float norm;
			for (int h = 0; h < numberOfGridPoints; h++)
			{
				norm = 0.0f;
				for (int i = 0; i < histogramBins; i++) {
					float val = desc[h*histogramBins + i];
					norm += val*val;
				}
				if (norm != 0.0) {
					norm = std::sqrt(norm);
					for (int i = 0; i < histogramBins; i++) {
						desc[h*histogramBins + i] /= norm;
					}
				}
			}
		}
		void Daisy::normalizeFull(Descriptor& desc)
		{
			float norm = 0.0f;
			for (int i = 0; i < (int)desc.size(); i++) {
				float val = desc[i];
				norm += val*val;
			}
			if (norm != 0.0) {
				norm = std::sqrt(norm);
				for (int i = 0; i < (int)desc.size(); i++) {
					desc[i] /= norm;
				}
			}
		}
		void Daisy::normalizeSiftWay(Descriptor& desc)
		{
			bool changed = true;
			int iter = 0;
			float norm;
			int h;
			const int MAX_NORMALIZATION_ITER = 5;
			const float m_descriptor_normalization_threshold = 0.154f; // sift magical number
			while (changed && iter < MAX_NORMALIZATION_ITER)
			{
				iter++;
				changed = false;
				norm = 0.0f;
				for (int i = 0; i < (int)desc.size(); i++) {
					float val = desc[i];
					norm += val*val;
				}
				norm = std::sqrt(norm);
				if (norm > 1e-5) {
					for (int i = 0; i < (int)desc.size(); i++) {
						desc[i] /= norm;
					}
				}
				for (h = 0; h < (int)desc.size(); h++)
				{
					if (desc[h] > m_descriptor_normalization_threshold)
					{
						desc[h] = m_descriptor_normalization_threshold;
						changed = true;
					}
				}
			}
		}
		void Daisy::getUnnormalizedDescriptor(float x, float y, int orientation, Descriptor& descriptor, bool disableInterpolation)
		{
			if (disableInterpolation) {
				ni_get_descriptor(x, y, orientation, descriptor);
			}
			else {
				i_get_descriptor(x, y, orientation, descriptor);
			}
		}
		void Daisy::getDescriptor(float x, float y, Descriptor& descriptor, int orientation, Normalization normalizationType, bool disableInterpolation) {
			getUnnormalizedDescriptor(x, y, orientation, descriptor, disableInterpolation);
			normalizeDescriptor(descriptor, normalizationType);
		}
		void Daisy::updateSelectedCubes() {
			selectedCubes.resize(radiusBins);
			for (int r = 0; r < radiusBins; r++) {
				float seed_sigma = (r + 1)*descriptorRadius / (radiusBins*0.5f);
				selectedCubes[r] = quantizeRadius(seed_sigma);
			}
		}
		void Daisy::computeCubeSigmas() {
			sigmas.resize(radiusBins);
			float r_step = descriptorRadius / radiusBins;
			for (int r = 0; r < radiusBins; r++) {
				sigmas[r] = (r + 1)*r_step / 2.0f;
			}
			updateSelectedCubes();
		}
		void Daisy::getDescriptors(DescriptorField& descriptorField, Normalization  normalizationType, bool scaleInvariant, bool rotationInvariant, bool disableInterpolation) {
			Image1f scaleMap;
			Image1i orientMap;
			if (scaleInvariant) {
				//Not really used
				computeScales(scaleMap);
			}
			if (rotationInvariant) {
				computeOrientations(orientMap, scaleMap, scaleInvariant);
			}
			//d::cout << "Scale " << scaleMap << std::endl;
			//WriteImageToRawFile(MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR << "scale.xml", scaleMap);

			//std::cout << "Orient " << orientMap << std::endl;
			//WriteImageToRawFile(MakeString() << GetDesktopDirectory() << ALY_PATH_SEPARATOR << "orient.xml", orientMap);

			//std::cout << "Write Descriptor" << std::endl;
			int orientation;
			descriptorField.resize(image.width, image.height);
#pragma omp parallel for
			for (int j = 0; j < image.height; j++) {
				for (int i = 0; i < image.width; i++) {
					orientation = 0;
					if (orientMap.size() > 0) orientation = orientMap(i, j).x;
					if (!(orientation >= 0 && orientation < ORIENTATIONS)) orientation = 0;
					getUnnormalizedDescriptor((float)i, (float)j, orientation, descriptorField(i, j), disableInterpolation);
					normalizeDescriptor(descriptorField(i, j), normalizationType);
				}
			}
		}
		void Daisy::computeOrientedGridPoints() {
			for (int i = 0; i < ORIENTATIONS; i++) {
				float angle = -i*2.0f*ALY_PI / ORIENTATIONS;
				float cosa = std::cos(angle);
				float sina = std::sin(angle);
				std::vector<float2>& point_list = orientedGridPoints[i];
				point_list.resize(numberOfGridPoints);
				for (int k = 0; k < numberOfGridPoints; k++) {
					float2 pt = gridPoints[k];
					point_list[k] = float2(pt.x*cosa + pt.y*sina, -pt.x*sina + pt.y*cosa);
				}
			}
		}
		inline double lengthL2(const Descriptor& a) {
			double ret = 0.0;
			int N = (int)a.size();
			for (int i = 0; i < N; i++) {
				ret += a[i] * a[i];
			}
			return std::sqrt(ret);
		}
		double lengthSqr(const Descriptor& a) {
			double ret = 0.0;
			int N = (int)a.size();
			for (int i = 0; i < N; i++) {
				ret += a[i] * a[i];
			}
			return ret;
		}
		double lengthL1(const Descriptor& a) {
			double ret = 0.0;
			int N = (int)a.size();
			for (int i = 0; i < N; i++) {
				ret += std::abs(a[i]);
			}
			return ret;
		}
		double angle(const Descriptor& a, const Descriptor& b) {
			return std::acos(dot(a, b) / (lengthL2(a)*lengthL2(b)));
		}
		void Daisy::computeGridPoints() {
			double r_step = descriptorRadius / radiusBins;
			double t_step = 2 * ALY_PI / angleBins;
			gridPoints.resize(numberOfGridPoints);
			for (int i = 0; i < numberOfGridPoints; i++) {
				gridPoints[i] = float2(0, 0);
			}
			for (int r = 0; r < radiusBins; r++) {
				int region = r*angleBins + 1;
				for (int t = 0; t < angleBins; t++) {
					float x, y;
					polar2cartesian((r + 1)*r_step, t*t_step, x, y);
					gridPoints[region + t] = float2(x, y);
				}
			}
			computeOrientedGridPoints();
		}
	}
}