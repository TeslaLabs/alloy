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
#include "AlloyUnits.h"
#include "AlloyContext.h"
namespace aly {
const Theme Theme::Default = Theme();
HSVA Color::toHSVA() const {
	HSVA hsva;
	float h, s, v;
	float min, max, delta;
	min = std::min(std::min(r, g), b);
	max = std::max(std::max(r, g), b);
	v = max;
	delta = max - min;
	if (max != 0) {
		s = delta / max;
	} else {
		return float4(0, 0, 0, a);
	}
	if (delta < 1E-6f)
		h = 0;
	else if (r == max)
		h = (g - b) / delta;
	else if (g == max)
		h = 2 + (b - r) / delta;
	else
		h = 4 + (r - g) / delta;
	h *= 60;
	if (h < 0)
		h += 360;
	return float4(h / 360.0f, s, v, a);
}
RGBA Color::toRGBA() const {
	return RGBA((int) clamp(255.0f * r, 0.0f, 255.0f), (int) clamp(255.0f * g, 0.0f, 255.0f), (int) clamp(255.0f * b, 0.0f, 255.0f),
			(int) clamp(255.0f * a, 0.0f, 255.0f));
}
HSV Color::toHSV() const {
	HSV hsva;
	float h, s, v;
	float min, max, delta;
	min = std::min(std::min(r, g), b);
	max = std::max(std::max(r, g), b);
	v = max;
	delta = max - min;

	if (max != 0) {
		s = delta / max;
	} else {
		return float3(0, 0, 0);
	}
	if (delta < 1E-6f)
		h = 0;
	else if (r == max)
		h = (g - b) / delta;
	else if (g == max)
		h = 2 + (b - r) / delta;
	else
		h = 4 + (r - g) / delta;
	h *= 60;
	if (h < 0)
		h += 360;
	return float3(h / 360.0f, s, v);
}
RGBf HSVtoRGBf(const HSV& hsv) {
	float h = hsv.x * 360.0f;
	float s = hsv.y;
	float v = hsv.z;
	int i;
	float f, p, q, t;
	if (s == 0) {
		return RGBf(v, v, v);
	}
	h /= 60.0f; // sector 0 to 5
	i = (int) std::floor(h);
	f = h - i;
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	switch (i) {
	case 0:
		return RGBf(v, t, p);
	case 1:
		return RGBf(q, v, p);
	case 2:
		return RGBf(p, v, t);
	case 3:
		return RGBf(p, q, v);
	case 4:
		return RGBf(t, p, v);
	default:
		return RGBf(v, p, q);
	}
}
float RGBtoGray(const float3& c, bool sRGB) {
	if (sRGB) {
		return (0.21f * c.x + 0.72f * c.y + 0.07f * c.z);
	} else {
		return (0.30f * c.x + 0.59f * c.y + 0.11f * c.z);
	}
}
float2 RGBAtoGray(const float4& c, bool sRGB) {
	if (sRGB) {
		return float2(0.21f * c.x + 0.72f * c.y + 0.07f * c.z, c.w);
	} else {
		return float2(0.30f * c.x + 0.59f * c.y + 0.11f * c.z, c.w);
	}
}
uint8_t RGBtoGray(const ubyte3& c, bool sRGB) {
	if (sRGB) {
		return (uint8_t) clamp(0.21f * c.x + 0.72f * c.y + 0.07f * c.z, 0.0f, 255.0f);
	} else {
		return (uint8_t) clamp(0.30f * c.x + 0.59f * c.y + 0.11f * c.z, 0.0f, 255.0f);
	}
}
ubyte2 RGBAtoGray(const ubyte4& c, bool sRGB) {
	if (sRGB) {
		return ubyte2((uint8_t) clamp(0.21f * c.x + 0.72f * c.y + 0.07f * c.z, 0.0f, 255.0f), c.w);
	} else {
		return ubyte2((uint8_t) clamp(0.30f * c.x + 0.59f * c.y + 0.11f * c.z, 0.0f, 255.0f), c.w);
	}
}
float3 GrayToRGB(float val) {
	return float3(val);
}
float4 GrayToRGBA(const float2& val) {
	return float4(val.x, val.x, val.x, val.y);
}
ubyte3 GrayToRGB(const uint8_t& val) {
	return ubyte3(val);
}
ubyte4 GrayToRGBA(const ubyte2& val) {
	return ubyte4(val.x, val.x, val.x, val.y);
}

float3 XYZtoRGB(const RGBf& xyz) {
	float x, y, z;
	x = xyz.x;
	y = xyz.y;
	z = xyz.z;
	float3 rgb;
	rgb.x = x * 3.2406f + y * -1.5372f + z * -0.4986f;
	rgb.y = x * -0.9689f + y * 1.8758f + z * 0.0415f;
	rgb.z = x * 0.0557f + y * -0.2040f + z * 1.0570f;
	if (rgb.x > 0.0031308)
		rgb.x = 1.055f * pow(rgb.x, (1.0f / 2.4f)) - 0.055f;
	else
		rgb.x = 12.92f * rgb.x;
	if (rgb.y > 0.0031308)
		rgb.y = 1.055f * pow(rgb.y, (1.0f / 2.4f)) - 0.055f;
	else
		rgb.y = 12.92f * rgb.y;
	if (rgb.z > 0.0031308)
		rgb.z = 1.055f * pow(rgb.z, (1.0f / 2.4f)) - 0.055f;
	else
		rgb.z = 12.92f * rgb.z;
	return aly::clamp(rgb, float3(0.0f), float3(1.0f));
}
float3 SRGBtoRGB(const float3& rgb) {
	float r = (rgb.x <= 0.04045f) ? rgb.x / 12.92f : pow((rgb.x + 0.055f) / 1.055f, 2.4f);
	float g = (rgb.y <= 0.04045f) ? rgb.y / 12.92f : pow((rgb.y + 0.055f) / 1.055f, 2.4f);
	float b = (rgb.z <= 0.04045f) ? rgb.z / 12.92f : pow((rgb.z + 0.055f) / 1.055f, 2.4f);
	return float3(r, g, b);
}
float3 RGBtoSRGB(const float3& rgb) {
	float r, g, b;
	if (rgb.x > 0.0031308)
		r = 1.055f * pow(rgb.x, (1.0f / 2.4f)) - 0.055f;
	else
		r = 12.92f * rgb.x;
	if (rgb.y > 0.0031308)
		g = 1.055f * pow(rgb.y, (1.0f / 2.4f)) - 0.055f;
	else
		g = 12.92f * rgb.y;
	if (rgb.z > 0.0031308)
		b = 1.055f * pow(rgb.z, (1.0f / 2.4f)) - 0.055f;
	else
		b = 12.92f * rgb.z;
	return float3(r, g, b);
}
float4 SRGBAtoRGBA(const float4& rgb) {
	float r = (rgb.x <= 0.04045f) ? rgb.x / 12.92f : pow((rgb.x + 0.055f) / 1.055f, 2.4f);
	float g = (rgb.y <= 0.04045f) ? rgb.y / 12.92f : pow((rgb.y + 0.055f) / 1.055f, 2.4f);
	float b = (rgb.z <= 0.04045f) ? rgb.z / 12.92f : pow((rgb.z + 0.055f) / 1.055f, 2.4f);
	return float4(r, g, b, rgb.w);
}
float4 RGBAtoSRGBA(const float4& rgb) {
	float r, g, b;
	if (rgb.x > 0.0031308)
		r = 1.055f * pow(rgb.x, (1.0f / 2.4f)) - 0.055f;
	else
		r = 12.92f * rgb.x;
	if (rgb.y > 0.0031308)
		g = 1.055f * pow(rgb.y, (1.0f / 2.4f)) - 0.055f;
	else
		g = 12.92f * rgb.y;
	if (rgb.z > 0.0031308)
		b = 1.055f * pow(rgb.z, (1.0f / 2.4f)) - 0.055f;
	else
		b = 12.92f * rgb.z;
	return float4(r, g, b, rgb.w);
}

float3 RGBtoXYZ(const RGBf& rgb) {
	float r, g, b;

	r = (rgb.x <= 0.04045f) ? rgb.x / 12.92f : pow((rgb.x + 0.055f) / 1.055f, 2.4f);
	g = (rgb.y <= 0.04045f) ? rgb.y / 12.92f : pow((rgb.y + 0.055f) / 1.055f, 2.4f);
	b = (rgb.z <= 0.04045f) ? rgb.z / 12.92f : pow((rgb.z + 0.055f) / 1.055f, 2.4f);

	float3 xyz;
	xyz.x = r * 0.4124564f + g * 0.3575761f + b * 0.1804375f;
	xyz.y = r * 0.2126729f + g * 0.7151522f + b * 0.0721750f;
	xyz.z = r * 0.0193339f + g * 0.1191920f + b * 0.9503041f;
	return xyz;
}
float3 RGBtoLAB(const float3& rgb) {
	float3 xyz = RGBtoXYZ(rgb);
	const float epsilon = 0.008856f;
	const float kappa = 903.3f;
	const float Xr = 0.950456f;
	const float Yr = 1.0f;
	const float Zr = 1.088754f;

	float xr = xyz.x / Xr;
	float yr = xyz.y / Yr;
	float zr = xyz.z / Zr;

	float fx, fy, fz;
	fx = (xr > epsilon) ? pow(xr, 1.0f / 3.0f) : (kappa * xr + 16.0f) / 116.0f;
	fy = (yr > epsilon) ? pow(yr, 1.0f / 3.0f) : (kappa * yr + 16.0f) / 116.0f;
	fz = (zr > epsilon) ? pow(zr, 1.0f / 3.0f) : (kappa * zr + 16.0f) / 116.0f;
	float3 lab;
	lab.x = (116.0f * fy - 16.0f);
	lab.y = (500.0f * (fx - fy));
	lab.z = (200.0f * (fy - fz));
	return lab;
}
float3 LABtoRGB(const float3& lab) {

	float xr = (lab.x + 16) / 116.0f + lab.y / 500.0f;
	float yr = (lab.x + 16) / 116.0f;
	float zr = (lab.x + 16) / 116.0f - lab.z / 200.0f;
	float epsilon = 6.0f / 29.0f;
	float fx = (xr > epsilon) ? pow(xr, 3.0f) : 3 * epsilon * epsilon * (xr - 4.0f / 29.0f);
	float fy = (yr > epsilon) ? pow(yr, 3.0f) : 3 * epsilon * epsilon * (yr - 4.0f / 29.0f);
	float fz = (zr > epsilon) ? pow(zr, 3.0f) : 3 * epsilon * epsilon * (zr - 4.0f / 29.0f);
	float3 xyz;
	const float Xr = 0.950456f;
	const float Yr = 1.0f;
	const float Zr = 1.088754f;
	xyz.x = Xr * fx;
	xyz.y = Yr * fy;
	xyz.z = Zr * fz;
	return XYZtoRGB(xyz);
}
float4 RGBAtoXYZA(const RGBAf& rgb) {
	return float4(RGBtoXYZ(rgb.xyz()), rgb.w);
}
float4 RGBAtoLABA(const float4& rgb) {
	return float4(RGBtoLAB(rgb.xyz()), rgb.w);
}
float4 XYZAtoRGBA(const RGBAf& rgb) {
	return float4(XYZtoRGB(rgb.xyz()), rgb.w);
}
float4 LABAtoRGBA(const float4& rgb) {
	return float4(LABtoRGB(rgb.xyz()), rgb.w);
}
RGBAf ColorMapToRGBAf(float x, const ColorMap& type) {
	static ImageRGBAf img;
	const int NUM_COLOR_MAPS = 12;
	if (img.size() == 0) {
		ReadImageFromFile(AlloyDefaultContext()->getFullPath("images/colormaps.png"), img);
	}
	int index = static_cast<int>(type);
	return img(clamp(x, 0.0f, 1.0f) * img.width, img.height * (index + 0.5f) / NUM_COLOR_MAPS);
}
RGBA ColorMapToRGBA(float x, const ColorMap& type) {
	return ToRGBA(ColorMapToRGBAf(x, type));
}
RGB ColorMapToRGB(float x, const ColorMap& type) {
	return ToRGB(ColorMapToRGBAf(x, type));
}
RGBf ColorMapToRGBf(float x, const ColorMap& type) {
	return ToRGBf(ColorMapToRGBAf(x, type));
}
RGBAf HSVAtoRGBAf(const HSVA& hsv) {
	float h = hsv.x * 360.0f;
	float s = hsv.y;
	float v = hsv.z;
	int i;
	float f, p, q, t;
	if (s == 0) {
		return RGBAf(v, v, v, hsv.w);
	}
	h /= 60.0f; // sector 0 to 5
	i = (int) std::floor(h);
	f = h - i;
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	switch (i) {
	case 0:
		return RGBAf(v, t, p, hsv.w);
	case 1:
		return RGBAf(q, v, p, hsv.w);
	case 2:
		return RGBAf(p, v, t, hsv.w);
	case 3:
		return RGBAf(p, q, v, hsv.w);
	case 4:
		return RGBAf(t, p, v, hsv.w);
	default:
		return RGBAf(v, p, q, hsv.w);
	}
}
Color HSVtoColor(const HSV& hsv) {
	float h = hsv.x * 360.0f;
	float s = hsv.y;
	float v = hsv.z;
	int i;
	float f, p, q, t;
	if (s == 0) {
		return Color(v, v, v);
	}
	h /= 60.0f; // sector 0 to 5
	i = (int) std::floor(h);
	f = h - i;
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	switch (i) {
	case 0:
		return Color(v, t, p);
	case 1:
		return Color(q, v, p);
	case 2:
		return Color(p, v, t);
	case 3:
		return Color(p, q, v);
	case 4:
		return Color(t, p, v);
	default:
		return Color(v, p, q);
	}
}
Color HSVAtoColor(const HSVA& hsva) {
	Color c = HSVtoColor(hsva.xyz());
	c.a = hsva.w;
	return c;
}
float3 Color::toXYZ() {
	return RGBtoXYZ(toRGBf());
}
float3 Color::toLAB() {
	return RGBtoLAB(toRGBf());
}
float4 Color::toXYZA() {
	return RGBAtoXYZA(toRGBAf());
}
float4 Color::toLABA() {
	return RGBAtoLABA(toRGBAf());
}
}

