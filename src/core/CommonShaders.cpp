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

#include <AlloyCamera.h>
#include "CommonShaders.h"
#include "AlloyMesh.h"
#include <set>
namespace aly {
ParticleDepthShader::ParticleDepthShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
			layout(location = 2) in vec4 vc;
			uniform float RADIUS;
			out VS_OUT {
				vec3 pos;
				float radius;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
				vs_out.radius=(RADIUS>0)?RADIUS:vc.w;
			}
		)",
			R"(
#version 330 core
in vec2 uv;
in vec4 vp;
in vec4 center;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
out vec4 FragColor;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		float r=center.w;
		vec4 pos=vp/vp.w;
		float rxy=length(pos-vec4(center.xyz,1.0));
		float cr=sqrt(r*r-rxy*rxy);
		float d=(-pos.z-cr-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
		vec3 norm=normalize(vec3(pos.x-center.x,pos.y-center.y,cr));
		FragColor=vec4(-norm,d);
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
	out vec4 vp;
    out vec4 center;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
		vec3 pos;
		float radius;
	} pc[];
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;
	void main() {
		mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		mat4 VM=ViewModelMat*PoseMat;
		vec4 pt = vec4(pc[0].pos,1.0);
		vec2 pos;
		float r = pc[0].radius;
		vec4 v = VM*pt;
		r = length(VM*vec4(0, 0, r, 0));
		center=vec4(v.xyz,r);

											vp=v + vec4(-r, -r, 0, 0);
		gl_Position  =ProjMat*(vp);
		uv = vec2(-1.0, -1.0);
		EmitVertex();
		vp=v + vec4(+r, -r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(1.0, -1.0);
		EmitVertex();
		vp=v + vec4(-r, +r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(-1.0, 1.0);
		EmitVertex();

											vp=v + vec4(+r, +r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();

												})");

}

void ParticleDepthShader::draw(const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& framebuffer, float radius) {
	framebuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set(camera,
			framebuffer.getViewport()).set("PoseMat", float4x4::identity()).draw(
			meshes, GLMesh::PrimitiveType::POINTS);
	end();
	glEnable(GL_BLEND);
	framebuffer.end();
}
void ParticleDepthShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& framebuffer, float radius) {
	framebuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set(camera,
			framebuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		set("PoseMat", pr.second).draw( { pr.first },
				GLMesh::PrimitiveType::POINTS);
	}
	end();
	glEnable(GL_BLEND);
	framebuffer.end();
}

void ParticleDepthShader::draw(const std::list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& framebuffer, float radius) {
	framebuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set("PoseMat",
			float4x4::identity()).set(camera, framebuffer.getViewport()).draw(
			meshes, GLMesh::PrimitiveType::POINTS).end();
	glEnable(GL_BLEND);
	framebuffer.end();
}
void ParticleDepthShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& framebuffer, float radius) {
	framebuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set(camera,
			framebuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		set("PoseMat", pr.second).draw( { pr.first },
				GLMesh::PrimitiveType::POINTS);
	}
	end();
	glEnable(GL_BLEND);
	framebuffer.end();
}
void ParticleMatcapShader::setTextureImage(const std::string& textureImage){
	matcapTexture.load(textureImage, false);
}
void ParticleMatcapShader::setTextureImage(const ImageRGBA& textureImage){
	matcapTexture.load(textureImage, false);
}
ParticleMatcapShader::ParticleMatcapShader(const std::string& textureImage,
		bool onScreen, const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context), matcapTexture(onScreen, context) {
	if(textureImage.size()>0)matcapTexture.load(textureImage, false);
	initialize( { },
			R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
			layout(location = 2) in vec4 vc;
			uniform float RADIUS;
			out VS_OUT {
				vec3 pos;
				vec4 color;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
				vs_out.color=vc;
			}
		)",
			R"(
#version 330 core
in vec2 uv;
in vec4 vp;
in vec4 color;
in vec4 center;
out vec4 FragColor;
uniform sampler2D matcapTexture;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		float r=center.w;
		vec4 pos=vp/vp.w;
		float rxy=length(pos-vec4(center.xyz,1.0));
		float cr=sqrt(r*r-rxy*rxy);
		float d=(-pos.z-cr-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
		vec3 norm=normalize(vec3(pos.x-center.x,pos.y-center.y,cr));
		FragColor=color*texture(matcapTexture,-0.5*norm.xy+0.5);
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
	out vec4 vp;
	out vec4 color;
    out vec4 center;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
		vec3 pos;
		vec4 color;
	} pc[];
	uniform float RADIUS;
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;

																	void main() {
		mat4 PVM=ProjMat*ViewModelMat*PoseMat;
		mat4 VM=ViewModelMat*PoseMat;
		vec4 pt = vec4(pc[0].pos,1.0);
		vec2 pos;
		vec4 vx;
		color=pc[0].color;
		float r = RADIUS;
		vec4 v = VM*pt;
		r = length(VM*vec4(0, 0, r, 0));
		center=vec4(v.xyz,r);

											vp=v + vec4(-r, -r, 0, 0);
		vx=ProjMat*(vp);
		vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);

											uv = vec2(-1.0, -1.0);
		EmitVertex();
		vp=v + vec4(+r, -r, 0, 0);
		vx=ProjMat*(vp);
                vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, -1.0);
		EmitVertex();
		vp=v + vec4(-r, +r, 0, 0);
		vx=ProjMat*(vp);
                vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(-1.0, 1.0);
		EmitVertex();

											vp=v + vec4(+r, +r, 0, 0);
		vx=ProjMat*(vp);
                vx=vx/vx.w;
		vx.x=0.5*(vx.x+1);
		vx.y=0.5*(1-vx.y);
		pos=vx.xy*bounds.zw+bounds.xy;
		gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
		uv = vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();

														})");

}

void ParticleMatcapShader::draw(
		const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, const box2px& bounds, const box2px& viewport,
		float radius) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) (bounds.dimensions.x),
			(int) (bounds.dimensions.y));
	CHECK_GL_ERROR();
	set("matcapTexture", matcapTexture, 0);
	CHECK_GL_ERROR();
	set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set("bounds", bounds).set(
			"viewport", viewport).set(camera, viewport).set("PoseMat",
			float4x4::identity());
	GLShader::draw(meshes, GLMesh::PrimitiveType::POINTS);
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();
}
void ParticleMatcapShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const box2px& bounds, const box2px& viewport,
		float radius) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) bounds.dimensions.x,
			(int) bounds.dimensions.y);
	set("MIN_DEPTH", camera.getNearPlane()).set("matcapTexture", matcapTexture,
			0).set("MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
			"bounds", bounds).set("viewport", viewport).set(camera, viewport);
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		set("PoseMat", pr.second).draw( { pr.first },
				GLMesh::PrimitiveType::POINTS);
	}
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();
}

void ParticleMatcapShader::draw(const std::list<Mesh*>& meshes,
		CameraParameters& camera, const box2px& bounds, const box2px& viewport,
		float radius) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) (bounds.dimensions.x),
			(int) (bounds.dimensions.y));

	set("MIN_DEPTH", camera.getNearPlane()).set("matcapTexture", matcapTexture,
			0).set("MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
			"bounds", bounds).set("viewport", viewport).set("PoseMat",
			float4x4::identity()).set(camera, viewport).draw(meshes,
			GLMesh::PrimitiveType::POINTS);
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();
}
void ParticleMatcapShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const box2px& bounds, const box2px& viewport,
		float radius) {
	begin();
	glEnable(GL_SCISSOR_TEST);
	glScissor((int) bounds.position.x,
			(int) (viewport.dimensions.y - bounds.position.y
					- bounds.dimensions.y), (int) (bounds.dimensions.x),
			(int) (bounds.dimensions.y));

	set("MIN_DEPTH", camera.getNearPlane()).set("matcapTexture", matcapTexture,
			0).set("MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
			"bounds", bounds).set("viewport", viewport).set(camera, viewport);
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		set("PoseMat", pr.second).draw( { pr.first },
				GLMesh::PrimitiveType::POINTS);
	}
	glScissor((int) viewport.position.x, (int) viewport.position.x,
			(int) viewport.dimensions.x, (int) viewport.dimensions.y);
	glDisable(GL_SCISSOR_TEST);
	end();
}
ParticleIdShader::ParticleIdShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context), framebuffer(onScreen, context) {
	GLShader::initialize( { },
			R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
			layout(location = 2) in vec4 vc;
			uniform float RADIUS;
			uniform int vertIdOffset;
			out VS_OUT {
				vec3 pos;
				float radius;
				int vertId;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
				vs_out.radius=(RADIUS>0)?RADIUS:vc.w;
				vs_out.vertId=int(gl_VertexID)+vertIdOffset;
			}
		)",
			R"(
#version 330 core
in vec2 uv;
in vec4 vp;
in vec4 center;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
flat in int vertId;
uniform int objectId;
out vec4 FragColor;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		float r=center.w;
		vec4 pos=vp/vp.w;
		float rxy=length(pos-vec4(center.xyz,1.0));
		float cr=sqrt(r*r-rxy*rxy);
		float d=(-pos.z-cr-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
		vec4 rgba = vec4(uint(vertId) & uint(0x00000FFF), ((uint(vertId) & uint(0x00FFF000)) >> uint(12)), ((uint(vertId) & uint(0xFF000000) ) >> uint(24)), 1+ objectId);
		FragColor = rgba;
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
	out vec4 vp;
    out vec4 center;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
		vec3 pos;
		float radius;
		int vertId;
	} pc[];

												flat out int vertId;
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;
	void main() {
		mat4 VM=ViewModelMat*PoseMat;
		vec4 pt = vec4(pc[0].pos,1.0);
		vec2 pos;
		float r = pc[0].radius;
		vec4 v = VM*pt;
		r = length(VM*vec4(0, 0, r, 0));
		center=vec4(v.xyz,r);
		vertId=pc[0].vertId;
		vp=v + vec4(-r, -r, 0, 0);
		gl_Position  =ProjMat*(vp);
		uv = vec2(-1.0, -1.0);
		EmitVertex();
		vp=v + vec4(+r, -r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(1.0, -1.0);
		EmitVertex();
		vp=v + vec4(-r, +r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(-1.0, 1.0);
		EmitVertex();

															vp=v + vec4(+r, +r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();

																})");

}
void ParticleIdShader::initialize(int w, int h) {
	framebuffer.initialize(w, h);
}
void ParticleIdShader::read(Image2i& faceIdMap) {
	faceIdMap.resize(framebuffer.width(), framebuffer.height());
	ImageRGBAf& irgba = framebuffer.getTexture().read();
	size_t idx = 0;
	int hash;
	int oid;
	for (RGBAf rgbaf : irgba.data) {
		int3 rgba = int3((int) rgbaf.x, (int) rgbaf.y, (int) rgbaf.z);
		if (rgbaf.w > 0.0f) {
			hash = (rgba.x) | (rgba.y << 12) | (rgba.z << 24);
			oid = ((int) std::floor(rgbaf.w + 0.5f)) - 1;
		} else {
			hash = -1;
			oid = -1;
		}
		faceIdMap[idx++] = int2(hash, oid);
	}
}
void ParticleIdShader::draw(const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, int faceIdOffset, int objectIdOffset,
		float radius) {
	framebuffer.begin();
	glDisable(GL_BLEND);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set("PoseMat",
			float4x4::identity()).set("vertIdOffset", faceIdOffset).set(camera,
			framebuffer.getViewport());
	for (Mesh* mesh : meshes) {
		set("objectId", objectIdOffset).draw(*mesh,
				GLMesh::PrimitiveType::POINTS);
		objectIdOffset++;
	}
	end();

	glEnable(GL_BLEND);
	framebuffer.end();
}
void ParticleIdShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, int faceIdOffset, int objectIdOffset,
		float radius) {
	framebuffer.begin();
	glDisable(GL_BLEND);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set("vertIdOffset",
			faceIdOffset).set(camera, framebuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		set("PoseMat", pr.second).set("objectId", objectIdOffset).draw(
				{ pr.first }, GLMesh::PrimitiveType::POINTS);
		objectIdOffset++;
	}
	end();
	glEnable(GL_BLEND);
	framebuffer.end();
}

void ParticleIdShader::draw(const std::list<Mesh*>& meshes,
		CameraParameters& camera, Image2i& faceIdMap, int faceIdOffset,
		int objectIdOffset, float radius) {
	faceIdMap.resize(framebuffer.width(), framebuffer.height());
	framebuffer.begin();
	glDisable(GL_BLEND);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set("PoseMat",
			float4x4::identity()).set("vertIdOffset", faceIdOffset).set(camera,
			framebuffer.getViewport());
	for (Mesh* mesh : meshes) {
		set("objectId", objectIdOffset).draw(*mesh,
				GLMesh::PrimitiveType::POINTS);
		objectIdOffset++;
	}
	end();
	glEnable(GL_BLEND);
	framebuffer.end();
	ImageRGBAf& irgba = framebuffer.getTexture().read();
	size_t idx = 0;
	int hash;
	int oid;
	for (RGBAf rgbaf : irgba.data) {
		int3 rgba = int3((int) rgbaf.x, (int) rgbaf.y, (int) rgbaf.z);
		if (rgbaf.w > 0.0f) {
			hash = (rgba.x) | (rgba.y << 12) | (rgba.z << 24);
			oid = ((int) std::floor(rgbaf.w + 0.5f)) - 1;
		} else {
			hash = -1;
			oid = -1;
		}
		faceIdMap[idx++] = int2(hash, oid);
	}
}
void ParticleIdShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, int faceIdOffset, int objectIdOffset,
		float radius) {
	framebuffer.begin();
	glDisable(GL_BLEND);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("RADIUS", radius).set("vertIdOffset",
			faceIdOffset).set(camera, framebuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		set("PoseMat", pr.second).set("objectId", objectIdOffset).draw(
				{ pr.first }, GLMesh::PrimitiveType::POINTS);
		objectIdOffset++;
	}
	end();
	glEnable(GL_BLEND);
	framebuffer.end();
}
SelectionShader::SelectionShader(bool onScreen,const std::shared_ptr<AlloyContext>& context) :
	GLShader(onScreen, context) {
	initialize({},
		R"(#version 330
		layout(location = 0) in vec3 vp;
		layout(location = 1) in vec2 vt;
		out vec2 uv;
		void main() {
			uv = vt;
			gl_Position = vec4(2*vp.x-1.0,1.0-2*vp.y, 0, 1);
		})",
		R"(
		#version 330
		in vec2 uv;
		uniform ivec4 faceId;
		uniform ivec2 depthBufferSize;
		uniform sampler2D depthImage;
		uniform sampler2D faceImage;
		out vec4 FragColor;
		void main() {
			ivec2 pos=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
			vec4 rgba=texelFetch(depthImage, pos, 0);
			vec4 faceIdIn=texelFetch(faceImage, pos, 0);
			int objectId=int(faceIdIn.w);
			if(faceId.w>=0&&objectId!=faceId.w){
				rgba=vec4(0.0,0.0,0.0,1.0);
			}
			if(faceId.x>=0&&(faceId.x!=faceIdIn.x||faceId.y!=faceIdIn.y||faceId.z!=faceIdIn.z)){
				rgba=vec4(0.0,0.0,0.0,1.0);
			}
			gl_FragDepth=rgba.w;
			FragColor=rgba;
		}
)");
}
CompositeShader::CompositeShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(
		 #version 330
		 layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
		 uniform vec4 bounds;
		 uniform vec4 viewport;
		out vec2 uv;
				 void main() {
		uv=vt;
		vec2 pos=vp.xy*bounds.zw+bounds.xy;
	    gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
			})",
			R"(
		 #version 330
		 uniform sampler2D sourceImage;
		 uniform sampler2D sourceDepth;
		 uniform sampler2D targetImage;
		 uniform sampler2D targetDepth;
		 in vec2 uv;
		 uniform float sourceAlpha;
		 uniform float targetAlpha;
out vec4 FragColor;
		 void main() {
		 vec4 srcColor=texture(sourceImage,uv);
         vec4 srcDepth=texture(sourceDepth,uv);
		 vec4 tarColor=texture(targetImage,uv);
         vec4 tarDepth=texture(targetDepth,uv);
         srcColor.w=srcColor.w*sourceAlpha;
         tarColor.w=tarColor.w*targetAlpha;
		 gl_FragDepth=min(srcDepth.w,tarDepth.w);
			if(tarDepth.w>=1.0&&srcDepth.w>=1.0){
				FragColor=vec4(0,0,0,0);
			} else if(tarDepth.w>0&&srcDepth.w<=0){
			   FragColor=tarColor;
			} else if(tarDepth.w<=0&&srcDepth.w>0){
			   FragColor=srcColor;
			} else if(srcDepth.w==tarDepth.w){
				FragColor=mix(srcColor,tarColor,srcColor.w/(srcColor.w+tarColor.w));
			} else if(srcDepth.w<tarDepth.w){
			   FragColor=mix(tarColor,srcColor,srcColor.w);
			}  else {
				FragColor=mix(srcColor,tarColor,tarColor.w);
			}
		 })");
}
void MatcapShader::setTextureImage(const std::string& textureImage){
	matcapTexture.load(textureImage, false);
}
void MatcapShader::setTextureImage(const ImageRGBA& textureImage){
	matcapTexture.load(textureImage, false);
}
MatcapShader::MatcapShader(const std::string& textureImage, bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context), matcapTexture(onScreen, context) {
	if(textureImage.size()>0)matcapTexture.load(textureImage, false);
	initialize( { },
			R"(
#version 330
layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
uniform vec4 bounds;
uniform vec4 viewport;
out vec2 uv;
void main() {
uv=vt;
vec2 pos=vp.xy*bounds.zw+bounds.xy;
gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
})",
			R"(
#version 330
in vec2 uv;
out vec4 FragColor;
uniform ivec2 depthBufferSize;
const float PI=3.1415926535;
uniform vec4 tint;
uniform sampler2D matcapTexture;
uniform sampler2D textureImage;
void main() {
ivec2 pos=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
vec4 rgba=texelFetch(textureImage, pos,0);//Do not interpolate depth buffer!
gl_FragDepth=rgba.w;
if(rgba.w<1.0){
  vec2 px=rgba.xy;
  px.x=-px.x;
  rgba=tint*texture(matcapTexture,0.5*px+0.5);
} else {
 rgba=vec4(0.0,0.0,0.0,0.0);
}
FragColor=rgba;
})");

}
TextureMeshShader::TextureMeshShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(
	#version 330
	layout(location = 0) in vec3 vp; 
	layout(location = 1) in vec2 vt; 
	uniform vec4 bounds;
	uniform vec4 viewport;
	out vec2 uv;
	void main() {
	uv=vt;
	vec2 pos=vp.xy*bounds.zw+bounds.xy;
	gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
	})",
			R"(
	#version 330
	in vec2 uv;
	uniform ivec2 depthBufferSize;
    uniform vec4 texBounds;
	uniform sampler2D textureImage;
	uniform sampler2D depthBuffer;
out vec4 FragColor;
	void main() {
	ivec2 pos=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
	vec4 rgba=texelFetch(depthBuffer, pos,0);//Do not interpolate depth buffer!
    vec2 pix=(rgba.xy-texBounds.xy)/(texBounds.zw);
    if(rgba.w<1.0&&pix.x>=0&&pix.y>=0&&pix.x<=1.0&&pix.y<=1){
		  pix.y=1.0-pix.y;
    	  vec4 c=texture(textureImage,pix);
          if(c.w>0){
              FragColor=c;
              gl_FragDepth=rgba.w;
          } else {
            discard;
          }
	} else {
	  discard;
	}

							})");

}
ImageShader::ImageShader( const Filter& filter,bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	if (filter == Filter::NONE) {
		initialize({},
			R"(
		 #version 330
		 layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
		 uniform vec4 bounds;
		 uniform vec4 viewport;
		uniform int flip;
		out vec2 uv;
				 void main() {
		if(flip!=0)uv=vec2(vt.x,1.0-vt.y); else uv=vt;
		vec2 pos=vp.xy*bounds.zw+bounds.xy;
	    gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
			})",
			R"(
		 #version 330
		 in vec2 uv;
		 out vec4 FragColor;
		 uniform sampler2D textureImage;
		 uniform float alpha;
		 void main() {
		 vec4 rgba=texture(textureImage,uv);
         rgba.w=rgba.w*alpha;
		 FragColor=rgba;
		 })");
	} else if (filter == Filter::SMALL_BLUR) {
		initialize( { },
				R"(
			 #version 330
			 layout(location = 0) in vec3 vp; 
	layout(location = 1) in vec2 vt; 
			 uniform vec4 bounds;
			 uniform vec4 viewport;
			uniform int flip;
			
			out vec2 uv;
					 void main() {
			if(flip!=0)uv=vec2(vt.x,1.0-vt.y); else uv=vt;
					 vec2 pos=vp.xy*bounds.zw+bounds.xy;
					 gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
				})",
				R"(
					 #version 330
			in vec2 uv;
			out vec4 FragColor;
			 uniform sampler2D textureImage;
			 void main() {
                vec4 colors[9];
				const float weights[9]=float[9](1,2,1,2,4,2,1,2,1);
                vec4 rgba;
				rgba =weights[0]*textureOffset(textureImage,uv, ivec2(-1,-1));
				rgba+=weights[1]*textureOffset(textureImage,uv, ivec2( 0,-1));
				rgba+=weights[2]*textureOffset(textureImage,uv, ivec2( 1,-1));
				rgba+=weights[3]*textureOffset(textureImage,uv, ivec2(-1, 0));
				rgba+=weights[4]*texture(textureImage,uv);
				rgba+=weights[5]*textureOffset(textureImage,uv, ivec2( 1, 0));
				rgba+=weights[6]*textureOffset(textureImage,uv, ivec2(-1, 1));
				rgba+=weights[7]*textureOffset(textureImage,uv, ivec2( 0, 1));
				rgba+=weights[8]*textureOffset(textureImage,uv, ivec2( 1, 1));
				FragColor=rgba/16.0;
			 })");
	} else if (filter == Filter::LARGE_BLUR) {
		initialize( { },
				R"(
			 #version 330
			 layout(location = 0) in vec3 vp; 
	layout(location = 1) in vec2 vt; 
			 uniform vec4 bounds;
			 uniform vec4 viewport;
			uniform int flip;
			out vec2 uv;
					 void main() {
			if(flip!=0)uv=vec2(vt.x,1.0-vt.y); else uv=vt;
					 vec2 pos=vp.xy*bounds.zw+bounds.xy;
					 gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
				})",
				R"(
					 #version 330
			 in vec2 uv;
			 uniform vec4 bounds;
			 uniform sampler2D textureImage;
			out vec4 FragColor;
			 void main() {
                vec4 colors[9];
				const float weights[25]=float[25](
					1,4,6,4,1,
					4,16,24,26,4,
					6,24,36,24,6,
					4,16,25,16,4,
					1,4,6,4,1);
                vec4 rgba=vec4(0,0,0,0);
float sum=0.0;
vec2 uvs;
				for(int i=-1;i<=1;i++){
					for(int j=-1;j<=1;j++){
                        uvs=uv+1.5f*vec2(i,j)/bounds.zw; 	
						rgba+=weights[0]*textureOffset(textureImage, uvs,  ivec2(-2,-2));
						rgba+=weights[1]*textureOffset(textureImage, uvs,  ivec2(-1,-2));
						rgba+=weights[2]*textureOffset(textureImage, uvs,  ivec2( 0,-2));
						rgba+=weights[3]*textureOffset(textureImage, uvs,  ivec2( 1,-2));
						rgba+=weights[4]*textureOffset(textureImage, uvs,  ivec2( 2,-2));

																		rgba+=weights[5]*textureOffset(textureImage, uvs,  ivec2(-2,-1));
						rgba+=weights[6]*textureOffset(textureImage, uvs,  ivec2(-1,-1));
						rgba+=weights[7]*textureOffset(textureImage, uvs,  ivec2( 0,-1));
						rgba+=weights[8]*textureOffset(textureImage, uvs,  ivec2( 1,-1));
						rgba+=weights[9]*textureOffset(textureImage, uvs,  ivec2( 2,-1));

																		rgba+=weights[10]*textureOffset(textureImage,uvs, ivec2(-2, 0));
						rgba+=weights[11]*textureOffset(textureImage,uvs, ivec2(-1, 0));
						rgba+=weights[12]*texture(textureImage,uvs);
						rgba+=weights[13]*textureOffset(textureImage,uvs, ivec2( 1, 0));
						rgba+=weights[14]*textureOffset(textureImage,uvs, ivec2( 2, 0));

																		rgba+=weights[15]*textureOffset(textureImage,uvs, ivec2(-2, 1));
						rgba+=weights[16]*textureOffset(textureImage,uvs, ivec2(-1, 1));
						rgba+=weights[17]*textureOffset(textureImage,uvs, ivec2( 0, 1));
						rgba+=weights[18]*textureOffset(textureImage,uvs, ivec2( 1, 1));
						rgba+=weights[19]*textureOffset(textureImage,uvs, ivec2( 2, 1));

																		rgba+=weights[20]*textureOffset(textureImage,uvs, ivec2(-2, 2));
						rgba+=weights[21]*textureOffset(textureImage,uvs, ivec2(-1, 2));
						rgba+=weights[22]*textureOffset(textureImage,uvs, ivec2( 0, 2));
						rgba+=weights[23]*textureOffset(textureImage,uvs, ivec2( 1, 2));
						rgba+=weights[24]*textureOffset(textureImage,uvs, ivec2( 2, 2));
sum+=256.0;
					}
				}

																FragColor=rgba/sum;
			 })");
	} else if (filter == Filter::MEDIUM_BLUR) {
		initialize( { },
				R"(
			 #version 330
			 layout(location = 0) in vec3 vp; 
	layout(location = 1) in vec2 vt; 
			 uniform vec4 bounds;
			 uniform vec4 viewport;
			uniform int flip;
			out vec2 uv;
					 void main() {
			if(flip!=0)uv=vec2(vt.x,1.0-vt.y); else uv=vt;
					 vec2 pos=vp.xy*bounds.zw+bounds.xy;
					 gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
				})",
				R"(
					 #version 330
			 in vec2 uv;
			 uniform vec4 bounds;
			 uniform sampler2D textureImage;
out vec4 FragColor;
			 void main() {
                vec4 colors[9];
				const float weights[25]=float[25](
					1,4,6,4,1,
					4,16,24,26,4,
					6,24,36,24,6,
					4,16,25,16,4,
					1,4,6,4,1);
                vec4 rgba;	
						rgba=weights[0]*textureOffset(textureImage,  uv,  ivec2(-2,-2));
						rgba+=weights[1]*textureOffset(textureImage, uv,  ivec2(-1,-2));
						rgba+=weights[2]*textureOffset(textureImage, uv,  ivec2( 0,-2));
						rgba+=weights[3]*textureOffset(textureImage, uv,  ivec2( 1,-2));
						rgba+=weights[4]*textureOffset(textureImage, uv,  ivec2( 2,-2));

																		rgba+=weights[5]*textureOffset(textureImage, uv,  ivec2(-2,-1));
						rgba+=weights[6]*textureOffset(textureImage, uv,  ivec2(-1,-1));
						rgba+=weights[7]*textureOffset(textureImage, uv,  ivec2( 0,-1));
						rgba+=weights[8]*textureOffset(textureImage, uv,  ivec2( 1,-1));
						rgba+=weights[9]*textureOffset(textureImage, uv,  ivec2( 2,-1));

																		rgba+=weights[10]*textureOffset(textureImage,uv, ivec2(-2, 0));
						rgba+=weights[11]*textureOffset(textureImage,uv, ivec2(-1, 0));
						rgba+=weights[12]*texture(textureImage,uv);
						rgba+=weights[13]*textureOffset(textureImage,uv, ivec2( 1, 0));
						rgba+=weights[14]*textureOffset(textureImage,uv, ivec2( 2, 0));

																		rgba+=weights[15]*textureOffset(textureImage,uv, ivec2(-2, 1));
						rgba+=weights[16]*textureOffset(textureImage,uv, ivec2(-1, 1));
						rgba+=weights[17]*textureOffset(textureImage,uv, ivec2( 0, 1));
						rgba+=weights[18]*textureOffset(textureImage,uv, ivec2( 1, 1));
						rgba+=weights[19]*textureOffset(textureImage,uv, ivec2( 2, 1));

																		rgba+=weights[20]*textureOffset(textureImage,uv, ivec2(-2, 2));
						rgba+=weights[21]*textureOffset(textureImage,uv, ivec2(-1, 2));
						rgba+=weights[22]*textureOffset(textureImage,uv, ivec2( 0, 2));
						rgba+=weights[23]*textureOffset(textureImage,uv, ivec2( 1, 2));
						rgba+=weights[24]*textureOffset(textureImage,uv, ivec2( 2, 2));
				FragColor=rgba/256.0;
			 })");
	} else if (filter == Filter::FXAA) {
		initialize( { },
				R"(
 #version 330
 layout(location = 0) in vec3 vp; 
 layout(location = 1) in vec2 vt; 
uniform float FXAA_SUBPIX_SHIFT = 1.0/4.0;
uniform vec4 bounds;
uniform vec4 viewport;
uniform int flip;
out vec4 posPos;
void main() {
 vec2 uv;
 if(flip!=0)uv=vec2(vt.x,1.0-vt.y); else uv=vt;
  posPos.xy = uv;
  vec2 rcpFrame = vec2(1.0/bounds.z, 1.0/bounds.w);
  posPos.zw = uv - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT));
  vec2 pos=vp.xy*bounds.zw+bounds.xy;
  gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
})",
				R"(
#version 330
uniform sampler2D tex0; // 0
out vec4 FragColor;
uniform vec4 bounds;
uniform float FXAA_SPAN_MAX = 8.0;
uniform float FXAA_REDUCE_MUL = 1.0/8.0;
in vec4 posPos;
#define FxaaInt2 ivec2
#define FxaaFloat2 vec2
#define FxaaTexLod0(t, p) textureLod(t, p, 0.0)
#define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)
 
vec3 FxaaPixelShader( 
  vec4 posPos, // Output of FxaaVertexShader interpolated across screen.
  sampler2D tex, // Input texture.
  vec2 rcpFrame) // Constant {1.0/frameWidth, 1.0/frameHeight}.
{   
/*---------------------------------------------------------*/
    #define FXAA_REDUCE_MIN   (1.0/128.0)
    //#define FXAA_REDUCE_MUL   (1.0/8.0)
    //#define FXAA_SPAN_MAX     8.0
/*---------------------------------------------------------*/
    vec3 rgbNW = FxaaTexLod0(tex, posPos.zw).xyz;
    vec3 rgbNE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1,0), rcpFrame.xy).xyz;
    vec3 rgbSW = FxaaTexOff(tex, posPos.zw, FxaaInt2(0,1), rcpFrame.xy).xyz;
    vec3 rgbSE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1,1), rcpFrame.xy).xyz;
    vec3 rgbM  = FxaaTexLod0(tex, posPos.xy).xyz;
/*---------------------------------------------------------*/
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
/*---------------------------------------------------------*/
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
/*---------------------------------------------------------*/
    vec2 dir; 
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
/*---------------------------------------------------------*/
    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(FxaaFloat2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
          max(FxaaFloat2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), 
          dir * rcpDirMin)) * rcpFrame.xy;
/*--------------------------------------------------------*/
    vec3 rgbA = (1.0/2.0) * (
        FxaaTexLod0(tex, posPos.xy + dir * (1.0/3.0 - 0.5)).xyz +
        FxaaTexLod0(tex, posPos.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        FxaaTexLod0(tex, posPos.xy + dir * (0.0/3.0 - 0.5)).xyz +
        FxaaTexLod0(tex, posPos.xy + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);
    if((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;
    return rgbB; }
 
vec4 PostFX(sampler2D tex, float time)
{
  vec4 c = vec4(0.0);
  vec2 rcpFrame = vec2(1.0/bounds.z, 1.0/bounds.w);
  c.rgb = FxaaPixelShader(posPos, tex, rcpFrame);
  c.a = texture(tex, posPos.xy).a;
  return c;
}
    
void main() 
{ 
  FragColor = PostFX(tex0, 0.0);
})");
	}
}

void FaceIdShader::initialize(int w, int h) {
	framebuffer.initialize(w, h);
}
int FaceIdShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, int faceIdOffset, int objectIdOffset,
		float radius) {
	glDisable(GL_BLEND);
	const bool flatShading = true;
	framebuffer.begin();
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, framebuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		int offset = faceIdOffset;
		set("objectId", objectIdOffset).set("PoseMat", pr.second);
		if (pr.first->triIndexes.size() == 0
				&& pr.first->quadIndexes.size() == 0) {
			end();
			particleIdShader.begin().set("MIN_DEPTH", camera.getNearPlane()).set(
					"MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
					camera, framebuffer.getViewport()).set("vertIdOffset",
					offset).set("PoseMat", pr.second).set("objectId",
					objectIdOffset).draw( { pr.first },
					GLMesh::PrimitiveType::POINTS).end();
			begin();
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("vertIdOffset", offset).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
		offset += (int) pr.first->triIndexes.size();
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("vertIdOffset", offset).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		objectIdOffset++;
	}
	end();
	framebuffer.end();
	return faceIdOffset;
}
int FaceIdShader::draw(const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, int faceIdOffset, int objectIdOffset,
		float radius) {
	glDisable(GL_BLEND);
	const bool flatShading = true;
	framebuffer.begin();
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			"PoseMat", float4x4::identity()).set(camera,
			framebuffer.getViewport());
	for (Mesh* mesh : meshes) {
		int offset = faceIdOffset;
		set("objectId", objectIdOffset);
		if (mesh->triIndexes.size() == 0 && mesh->quadIndexes.size() == 0) {
			end();
			particleIdShader.begin().set("MIN_DEPTH", camera.getNearPlane()).set(
					"MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
					camera, framebuffer.getViewport()).set("vertIdOffset",
					offset).set("PoseMat", float4x4::identity()).set("objectId",
					objectIdOffset).draw( { mesh },
					GLMesh::PrimitiveType::POINTS).end();
			begin();
		}
		if (mesh->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("vertIdOffset", offset).draw( { mesh },
					GLMesh::PrimitiveType::TRIANGLES);
		}
		offset += (int) mesh->triIndexes.size();
		if (mesh->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("vertIdOffset", offset).draw( { mesh },
					GLMesh::PrimitiveType::QUADS);
		}
		objectIdOffset++;
	}
	end();
	framebuffer.end();
	glEnable(GL_BLEND);
	return faceIdOffset;
}
int FaceIdShader::draw(
	const std::vector<std::pair<Mesh*, float4x4>>& meshes,
	CameraParameters& camera, int faceIdOffset, int objectIdOffset,
	float radius) {
	glDisable(GL_BLEND);
	const bool flatShading = true;
	framebuffer.begin();
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
		flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, framebuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		int offset = faceIdOffset;
		set("objectId", objectIdOffset).set("PoseMat", pr.second);
		if (pr.first->triIndexes.size() == 0
			&& pr.first->quadIndexes.size() == 0) {
			end();
			particleIdShader.begin().set("MIN_DEPTH", camera.getNearPlane()).set(
				"MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
					camera, framebuffer.getViewport()).set("vertIdOffset",
						offset).set("PoseMat", pr.second).set("objectId",
							objectIdOffset).draw({ pr.first },
								GLMesh::PrimitiveType::POINTS).end();
			begin();
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("vertIdOffset", offset).draw({ pr.first },
				GLMesh::PrimitiveType::TRIANGLES);
		}
		offset += (int)pr.first->triIndexes.size();
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("vertIdOffset", offset).draw({ pr.first },
				GLMesh::PrimitiveType::QUADS);
		}
		objectIdOffset++;
	}
	end();
	framebuffer.end();
	glEnable(GL_BLEND);
	return faceIdOffset;
}
int FaceIdShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, int faceIdOffset, int objectIdOffset,
		float radius) {
	glDisable(GL_BLEND);
	const bool flatShading = true;
	framebuffer.begin();
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, framebuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		int offset = faceIdOffset;
		set("objectId", objectIdOffset).set("PoseMat", pr.second);
		if (pr.first->triIndexes.size() == 0
				&& pr.first->quadIndexes.size() == 0) {
			end();
			particleIdShader.begin().set("MIN_DEPTH", camera.getNearPlane()).set(
					"MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
					camera, framebuffer.getViewport()).set("vertIdOffset",
					offset).set("PoseMat", pr.second).set("objectId",
					objectIdOffset).draw( { pr.first },
					GLMesh::PrimitiveType::POINTS).end();
			begin();
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("vertIdOffset", offset).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
		offset += (int) pr.first->triIndexes.size();
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("vertIdOffset", offset).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		objectIdOffset++;
	}
	end();
	framebuffer.end();
	glEnable(GL_BLEND);
	return faceIdOffset;
}
void FaceIdShader::read(Image2i& faceIdMap) {
	faceIdMap.resize(framebuffer.width(), framebuffer.height());
	ImageRGBAf& irgba = framebuffer.getTexture().read();
	size_t idx = 0;
	int hash;
	int oid;
	for (RGBAf rgbaf : irgba.data) {
		int3 rgba = int3((int) rgbaf.x, (int) rgbaf.y, (int) rgbaf.z);
		if (rgbaf.w > 0.0f) {
			hash = (rgba.x) | (rgba.y << 12) | (rgba.z << 24);
			oid = ((int) std::floor(rgbaf.w + 0.5f)) - 1;
		} else {
			hash = -1;
			oid = -1;
		}
		faceIdMap[idx++] = int2(hash, oid);
	}

}
int FaceIdShader::draw(const std::list<Mesh*>& meshes,
		CameraParameters& camera, int faceIdOffset, int objectIdOffset,
		float radius) {
	glDisable(GL_BLEND);
	const bool flatShading = true;
	framebuffer.begin();
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			"PoseMat", float4x4::identity()).set(camera,
			framebuffer.getViewport());
	for (Mesh* mesh : meshes) {
		int offset = faceIdOffset;
		set("objectId", objectIdOffset);
		if (mesh->triIndexes.size() == 0 && mesh->quadIndexes.size() == 0) {
			end();
			particleIdShader.begin().set("MIN_DEPTH", camera.getNearPlane()).set(
					"MAX_DEPTH", camera.getFarPlane()).set("RADIUS", radius).set(
					camera, framebuffer.getViewport()).set("vertIdOffset",
					offset).set("PoseMat", float4x4::identity()).set("objectId",
					objectIdOffset).draw( { mesh },
					GLMesh::PrimitiveType::POINTS).end();
			begin();
		}
		if (mesh->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("vertIdOffset", offset).draw( { mesh },
					GLMesh::PrimitiveType::TRIANGLES);
		}
		offset += (int) mesh->triIndexes.size();
		if (mesh->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("vertIdOffset", offset).draw( { mesh },
					GLMesh::PrimitiveType::QUADS);
		}
		objectIdOffset++;
	}
	end();
	framebuffer.end();
	glEnable(GL_BLEND);
	return faceIdOffset;
}

FaceIdShader::FaceIdShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context), framebuffer(onScreen, context), particleIdShader(
				onScreen, context) {
	GLShader::initialize( { },
			R"(	#version 330
				layout(location = 3) in vec3 vp0;
				layout(location = 4) in vec3 vp1;
				layout(location = 5) in vec3 vp2;
				layout(location = 6) in vec3 vp3;
				layout(location = 7) in vec3 vn0;
				layout(location = 8) in vec3 vn1;
				layout(location = 9) in vec3 vn2;
				layout(location = 10) in vec3 vn3;
				uniform int vertIdOffset;
				out VS_OUT {
					vec3 p0;
					vec3 p1;
					vec3 p2;
					vec3 p3;
					vec3 n0;
					vec3 n1;
					vec3 n2;
					vec3 n3;
					int vertId;
				} vs_out;
				void main(void) {
					vs_out.p0=vp0;
					vs_out.p1=vp1;
					vs_out.p2=vp2;
					vs_out.p3=vp3;
					vs_out.n0=vn0;
					vs_out.n1=vn1;
					vs_out.n2=vn2;
					vs_out.n3=vn3;
					vs_out.vertId=int(gl_VertexID)+vertIdOffset;
				}
)",
			R"(
	#version 330
	flat in int vertId;
	uniform int objectId;
	out vec4 FragColor;
	void main() {
		vec4 rgba = vec4(uint(vertId) & uint(0x00000FFF), ((uint(vertId) & uint(0x00FFF000)) >> uint(12)), ((uint(vertId) & uint(0xFF000000) ) >> uint(24)), 1+ objectId);
		FragColor = rgba;
}
	)",
			R"(	#version 330
					layout (points) in;
					layout (triangle_strip, max_vertices=4) out;
					in VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
						vec3 n0;
						vec3 n1;
						vec3 n2;
						vec3 n3;
						int vertId;
					} quad[];
					out vec3 v0, v1, v2, v3;
					out vec3 normal;
					out vec3 vert;
					flat out int vertId;
					uniform int IS_QUAD;
                    uniform int IS_FLAT;
				uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
					void main() {
					  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
					  mat4 VM=ViewModelMat*PoseMat;
					  vertId=quad[0].vertId;
					  vec3 p0=quad[0].p0;
					  vec3 p1=quad[0].p1;
					  vec3 p2=quad[0].p2;
                      vec3 p3=quad[0].p3;
					
					  v0 = (VM*vec4(p0,1)).xyz;
					  v1 = (VM*vec4(p1,1)).xyz;
					  v2 = (VM*vec4(p2,1)).xyz;
                      v3 = (VM*vec4(p3,1)).xyz;
					  
		
if(IS_QUAD!=0){
	gl_Position=PVM*vec4(p0,1);  
	vert = v0;
	if(IS_FLAT!=0){
        vec3 pt=0.25*(p0+p1+p2+p3);
        normal = cross(p0-pt, p1-pt)+cross(p1-pt, p2-pt)+cross(p2-pt, p3-pt)+cross(p3-pt, p0-pt);
		normal = (VM*vec4(normalize(-normal),0.0)).xyz;
	} else {
		normal= (VM*vec4(quad[0].n0,0.0)).xyz;
	}
	EmitVertex();
} else {
	gl_Position=PVM*vec4(p0,1);  
	vert = v0;
	if(IS_FLAT!=0){
		normal = (VM*vec4(normalize(cross( p2-p0, p1-p0)),0.0)).xyz;
	} else {
		normal= (VM*vec4(quad[0].n0,0.0)).xyz;
	}
	EmitVertex();
}
	gl_Position=PVM*vec4(p1,1);  
	vert = v1;
	if(IS_FLAT!=0){
		//normal = (VM*vec4(normalize(cross( p0-p1, p2-p1)),0.0)).xyz;
	} else {
		normal= (VM*vec4(quad[0].n1,0.0)).xyz;
	}
	EmitVertex();
	if(IS_QUAD!=0){
		gl_Position=PVM*vec4(p3,1);  
		vert = v3;
		if(IS_FLAT!=0){
			//normal = (VM*vec4(normalize(cross( p2-p3, p0-p3)),0.0)).xyz;
		} else {
			normal= (VM*vec4(quad[0].n3,0.0)).xyz;
		}
		EmitVertex();
		gl_Position=PVM*vec4(p2,1);  
		vert = v2;
		if(IS_FLAT!=0){
			//normal = (VM*vec4(normalize(cross( p1-p2, p3-p2)),0.0)).xyz;
		} else {
			normal= (VM*vec4(quad[0].n2,0.0)).xyz;
		}
		EmitVertex();
		EndPrimitive();
	} else {
		gl_Position=PVM*vec4(p2,1);  
		vert = v2;
		if(IS_FLAT!=0){	
			//normal = (VM*vec4(normalize(cross( p1-p2, p0-p2)),0.0)).xyz;
		} else {
			normal= (VM*vec4(quad[0].n2,0.0)).xyz;
		}
		EmitVertex();
		EndPrimitive();
	}
})");
particleIdShader.initialize({},
	R"(
			#version 330 core
			#extension GL_ARB_separate_shader_objects : enable
			layout(location = 0) in vec3 vp;
			layout(location = 2) in vec4 vc;
			uniform float RADIUS;
			uniform int vertIdOffset;
			out VS_OUT {
				vec3 pos;
				float radius;
				int vertId;
			} vs_out;
			void main(void) {
				vs_out.pos=vp;
				vs_out.radius=(RADIUS>0)?RADIUS:vc.w;
				vs_out.vertId=int(gl_VertexID)+vertIdOffset;
			}
		)",
	R"(
#version 330 core
in vec2 uv;
in vec4 vp;
in vec4 center;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
flat in int vertId;
uniform int objectId;
out vec4 FragColor;
void main(void) {
	float radius=length(uv);
	if(radius>1.0){
		discard;
	} else {
		float r=center.w;
		vec4 pos=vp/vp.w;
		float rxy=length(pos-vec4(center.xyz,1.0));
		float cr=sqrt(r*r-rxy*rxy);
		float d=(-pos.z-cr-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
		vec4 rgba = vec4(uint(vertId) & uint(0x00000FFF), ((uint(vertId) & uint(0x00FFF000)) >> uint(12)), ((uint(vertId) & uint(0xFF000000) ) >> uint(24)), 1+ objectId);
		FragColor = rgba;
		gl_FragDepth=d;
	}
}
)",
			R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
	out vec2 uv;
	out vec4 vp;
    out vec4 center;
out vec4 FragColor;
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;
	in VS_OUT {
		vec3 pos;
		float radius;
		int vertId;
	} pc[];

											flat out int vertId;
	uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
	uniform vec4 bounds;
	uniform vec4 viewport;
	void main() {
		mat4 VM=ViewModelMat*PoseMat;
		vec4 pt = vec4(pc[0].pos,1.0);
		vec2 pos;
		float r = pc[0].radius;
		vec4 v = VM*pt;
		r = length(VM*vec4(0, 0, r, 0));
		center=vec4(v.xyz,r);
		vertId=pc[0].vertId;
		vp=v + vec4(-r, -r, 0, 0);
		gl_Position  =ProjMat*(vp);
		uv = vec2(-1.0, -1.0);
		EmitVertex();
		vp=v + vec4(+r, -r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(1.0, -1.0);
		EmitVertex();
		vp=v + vec4(-r, +r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(-1.0, 1.0);
		EmitVertex();

														vp=v + vec4(+r, +r, 0, 0);
		gl_Position  =ProjMat*(vp);
				uv = vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();

															})");
}
DepthAndNormalShader::DepthAndNormalShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize({},
		R"(	#version 330
				layout(location = 3) in vec3 vp0;
				layout(location = 4) in vec3 vp1;
				layout(location = 5) in vec3 vp2;
				layout(location = 6) in vec3 vp3;

															layout(location = 7) in vec3 vn0;
				layout(location = 8) in vec3 vn1;
				layout(location = 9) in vec3 vn2;
				layout(location = 10) in vec3 vn3;
	
				out VS_OUT {
					vec3 p0;
					vec3 p1;
					vec3 p2;
					vec3 p3;
					vec3 n0;
					vec3 n1;
					vec3 n2;
					vec3 n3;
				} vs_out;
				void main(void) {
					vs_out.p0=vp0;
					vs_out.p1=vp1;
					vs_out.p2=vp2;
					vs_out.p3=vp3;
					vs_out.n0=vn0;
					vs_out.n1=vn1;
					vs_out.n2=vn2;
					vs_out.n3=vn3;
				})",
		R"(	#version 330
					in vec3 normal;
					in vec3 vert;
					uniform float MIN_DEPTH;
					uniform float MAX_DEPTH;
					out vec4 FragColor;
					void main() {
						vec3 normalized_normal = normalize(normal);
						FragColor = vec4(normalized_normal.xyz,(-vert.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH));
					}
					)",
			R"(	#version 330
					layout (points) in;
					layout (triangle_strip, max_vertices=4) out;
					in VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
						vec3 n0;
						vec3 n1;
						vec3 n2;
						vec3 n3;
					} quad[];
					out vec3 v0, v1, v2, v3;
					out vec3 normal;
					out vec3 vert;
					uniform int IS_QUAD;
                    uniform int IS_FLAT;
				uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
					void main() {
					  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
					  mat4 VM=ViewModelMat*PoseMat;
					  
					  vec3 p0=quad[0].p0;
					  vec3 p1=quad[0].p1;
					  vec3 p2=quad[0].p2;
                      vec3 p3=quad[0].p3;
					
					  v0 = (VM*vec4(p0,1)).xyz;
					  v1 = (VM*vec4(p1,1)).xyz;
					  v2 = (VM*vec4(p2,1)).xyz;
                      v3 = (VM*vec4(p3,1)).xyz;
					  
					  
if(IS_QUAD!=0){
	gl_Position=PVM*vec4(p0,1);  
	vert = v0;
	if(IS_FLAT!=0){
        vec3 pt=0.25*(p0+p1+p2+p3);
        normal = cross(p0-pt, p1-pt)+cross(p1-pt, p2-pt)+cross(p2-pt, p3-pt)+cross(p3-pt, p0-pt);
		normal = (VM*vec4(normalize(-normal),0.0)).xyz;
	} else {
		normal= (VM*vec4(quad[0].n0,0.0)).xyz;
	}
	EmitVertex();
} else {
	gl_Position=PVM*vec4(p0,1);  
	vert = v0;
	if(IS_FLAT!=0){
		normal = (VM*vec4(normalize(cross( p2-p0, p1-p0)),0.0)).xyz;
	} else {
		normal= (VM*vec4(quad[0].n0,0.0)).xyz;
	}
	EmitVertex();
}
	gl_Position=PVM*vec4(p1,1);  
	vert = v1;
	if(IS_FLAT!=0){
		//normal = (VM*vec4(normalize(cross( p0-p1, p2-p1)),0.0)).xyz;
	} else {
		normal= (VM*vec4(quad[0].n1,0.0)).xyz;
	}
	EmitVertex();
	if(IS_QUAD!=0){
		gl_Position=PVM*vec4(p3,1);  
		vert = v3;
		if(IS_FLAT!=0){
			//normal = (VM*vec4(normalize(cross( p2-p3, p0-p3)),0.0)).xyz;
		} else {
			normal= (VM*vec4(quad[0].n3,0.0)).xyz;
		}
		EmitVertex();
		gl_Position=PVM*vec4(p2,1);  
		vert = v2;
		if(IS_FLAT!=0){
			//normal = (VM*vec4(normalize(cross( p1-p2, p3-p2)),0.0)).xyz;
		} else {
			normal= (VM*vec4(quad[0].n2,0.0)).xyz;
		}
		EmitVertex();
		EndPrimitive();
	} else {
		gl_Position=PVM*vec4(p2,1);  
		vert = v2;
		if(IS_FLAT!=0){
			//normal = (VM*vec4(normalize(cross( p1-p2, p0-p2)),0.0)).xyz;
		} else {
			normal= (VM*vec4(quad[0].n2,0.0)).xyz;
		}
		EmitVertex();
		EndPrimitive();
	}
})");

}
void DepthAndNormalShader::draw(
		const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport()).set("PoseMat",
			float4x4::identity());
	set("IS_QUAD", 1).draw(meshes, GLMesh::PrimitiveType::QUADS);
	set("IS_QUAD", 0).draw(meshes, GLMesh::PrimitiveType::TRIANGLES);
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void DepthAndNormalShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void DepthAndNormalShader::draw(const std::list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport()).set("PoseMat",
			float4x4::identity());
	set("IS_QUAD", 1).draw(meshes, GLMesh::PrimitiveType::QUADS);
	set("IS_QUAD", 0).draw(meshes, GLMesh::PrimitiveType::TRIANGLES);
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void DepthAndNormalShader::draw(const std::vector<Mesh*>& meshes,
	CameraParameters& camera, GLFrameBuffer& frameBuffer,
	bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
		flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport()).set("PoseMat",
				float4x4::identity());
	set("IS_QUAD", 1).draw(meshes, GLMesh::PrimitiveType::QUADS);
	set("IS_QUAD", 0).draw(meshes, GLMesh::PrimitiveType::TRIANGLES);
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void DepthAndNormalShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void DepthAndNormalShader::draw(
	const std::vector<std::pair<Mesh*, float4x4>>& meshes,
	CameraParameters& camera, GLFrameBuffer& frameBuffer,
	bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
		flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw({ pr.first },
				GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw({ pr.first },
				GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
ColorVertexShader::ColorVertexShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(	#version 330
				layout(location = 3) in vec3 vp0;
				layout(location = 4) in vec3 vp1;
				layout(location = 5) in vec3 vp2;
				layout(location = 6) in vec3 vp3;
	
				layout(location = 11) in vec4 vc0;
				layout(location = 12) in vec4 vc1;
				layout(location = 13) in vec4 vc2;
				layout(location = 14) in vec4 vc3;
				out VS_OUT {
					vec3 p0;
					vec3 p1;
					vec3 p2;
					vec3 p3;
					vec4 c0;
					vec4 c1;
					vec4 c2;
					vec4 c3;
				} vs_out;
				void main(void) {
					vs_out.p0=vp0;
					vs_out.p1=vp1;
					vs_out.p2=vp2;
					vs_out.p3=vp3;
					vs_out.c0=vc0;
					vs_out.c1=vc1;
					vs_out.c2=vc2;
					vs_out.c3=vc3;
				})",
			R"(	#version 330
                    in vec4 color;
					out vec4 FragColor;
					void main() {
						FragColor = color;
					}
					)",
			R"(	#version 330
					layout (points) in;
					layout (triangle_strip, max_vertices=4) out;
					in VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
						vec4 c0;
						vec4 c1;
						vec4 c2;
						vec4 c3;
					} quad[];
					vec3 v0, v1, v2, v3;
                    vec4 c0, c1, c2, c3;
                    out vec4 color;
					uniform int IS_QUAD;
                    uniform int IS_FLAT;
				uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
					void main() {
					  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
					  mat4 VM=ViewModelMat*PoseMat;
					  
					  vec3 p0=quad[0].p0;
					  vec3 p1=quad[0].p1;
					  vec3 p2=quad[0].p2;
                      vec3 p3=quad[0].p3;
					
                      c0 = quad[0].c0;
					  c1 = quad[0].c1;
					  c2 = quad[0].c2;
					  c3 = quad[0].c3;

											  v0 = (VM*vec4(p0,1)).xyz;
					  v1 = (VM*vec4(p1,1)).xyz;
					  v2 = (VM*vec4(p2,1)).xyz;
                      v3 = (VM*vec4(p3,1)).xyz;
			vec4 avg;		  
if(IS_FLAT!=0){
	if(IS_QUAD!=0){
		avg=0.25*(c0+c1+c2+c3);
	} else {
		avg=0.333333*(c0+c1+c2);
	}
    c0=c1=c2=c3=avg;	
}
					  
if(IS_QUAD!=0){
	gl_Position=PVM*vec4(p0,1);  
    color=c0;
	EmitVertex();
} else {
	gl_Position=PVM*vec4(p0,1);  
    color=c0;
	EmitVertex();
}
	gl_Position=PVM*vec4(p1,1);  
    color=c1;
	EmitVertex();
	if(IS_QUAD!=0){
		gl_Position=PVM*vec4(p3,1);  
        color=c3;
		EmitVertex();
		gl_Position=PVM*vec4(p2,1);  
        color=c2;
		EmitVertex();
		EndPrimitive();
	} else {
		gl_Position=PVM*vec4(p2,1);  
		color=c2;
		EmitVertex();
		EndPrimitive();
	}
})");

}
void ColorVertexShader::draw(const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport()).set("PoseMat",
			float4x4::identity());
	set("IS_QUAD", 1).draw(meshes, GLMesh::PrimitiveType::QUADS, true);
	set("IS_QUAD", 0).draw(meshes, GLMesh::PrimitiveType::TRIANGLES, true);
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void ColorVertexShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS, true);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES, true);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void ColorVertexShader::draw(const std::list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport()).set("PoseMat",
			float4x4::identity());
	set("IS_QUAD", 1).draw(meshes, GLMesh::PrimitiveType::QUADS, true);
	set("IS_QUAD", 0).draw(meshes, GLMesh::PrimitiveType::TRIANGLES, true);
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void ColorVertexShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS, true);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES, true);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}

DepthAndTextureShader::DepthAndTextureShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(	#version 330
				layout(location = 3) in vec3 vp0;
				layout(location = 4) in vec3 vp1;
				layout(location = 5) in vec3 vp2;
				layout(location = 6) in vec3 vp3;

										layout(location = 11) in vec2 vt0;
				layout(location = 12) in vec2 vt1;
				layout(location = 13) in vec2 vt2;
				layout(location = 14) in vec2 vt3;
	
				out VS_OUT {
					vec3 p0;
					vec3 p1;
					vec3 p2;
					vec3 p3;
					vec2 t0;
					vec2 t1;
					vec2 t2;
					vec2 t3;
				} vs_out;
				void main(void) {
					vs_out.p0=vp0;
					vs_out.p1=vp1;
					vs_out.p2=vp2;
					vs_out.p3=vp3;
					vs_out.t0=vt0;
					vs_out.t1=vt1;
					vs_out.t2=vt2;
					vs_out.t3=vt3;
				})",
			R"(	#version 330
					in vec3 vert;
					in vec2 tex;
					uniform float MIN_DEPTH;
					uniform float MAX_DEPTH;
					out vec4 FragColor;
					void main() {
						FragColor = vec4(tex.x,tex.y,0.0,(-vert.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH));
					}
					)",
			R"(	#version 330
					layout (points) in;
					layout (triangle_strip, max_vertices=4) out;
					in VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
						vec2 t0;
						vec2 t1;
						vec2 t2;
						vec2 t3;
					} quad[];
					out vec3 v0, v1, v2, v3;
					out vec2 tex;
					out vec3 vert;
					uniform int IS_QUAD;
				uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
					void main() {
					  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
					  mat4 VM=ViewModelMat*PoseMat;
					  
					  vec3 p0=quad[0].p0;
					  vec3 p1=quad[0].p1;
					  vec3 p2=quad[0].p2;
                      vec3 p3=quad[0].p3;
					
					  v0 = (VM*vec4(p0,1)).xyz;
					  v1 = (VM*vec4(p1,1)).xyz;
					  v2 = (VM*vec4(p2,1)).xyz;
                      v3 = (VM*vec4(p3,1)).xyz;
					  					  
	gl_Position=PVM*vec4(p0,1);  
	vert = v0;
	tex = quad[0].t0;
	EmitVertex();

										gl_Position=PVM*vec4(p1,1);  
	vert = v1;
	tex = quad[0].t1;
	EmitVertex();

										if(IS_QUAD!=0){
		gl_Position=PVM*vec4(p3,1);  
		vert = v3;
		tex = quad[0].t3;
		EmitVertex();
		gl_Position=PVM*vec4(p2,1);  
		vert = v2;
		tex = quad[0].t2;
		EmitVertex();
		EndPrimitive();
	} else {
		gl_Position=PVM*vec4(p2,1);  
		vert = v2;
		tex = quad[0].t2;
		EmitVertex();
		EndPrimitive();
	}
})");

}
void DepthAndTextureShader::draw(
		const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin();
	set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT", flatShading ? 1 : 0).set(
			"MAX_DEPTH", camera.getFarPlane()).set(camera,
			frameBuffer.getViewport()).set("PoseMat", float4x4::identity());
	set("IS_QUAD", 1).draw(meshes, GLMesh::PrimitiveType::QUADS);
	set("IS_QUAD", 0).draw(meshes, GLMesh::PrimitiveType::TRIANGLES);
	glEnable(GL_BLEND);
	end();
	frameBuffer.end();
}
void DepthAndTextureShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void DepthAndTextureShader::draw(const std::list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport()).set("PoseMat",
			float4x4::identity());
	set("IS_QUAD", 1).draw(meshes, GLMesh::PrimitiveType::QUADS);
	set("IS_QUAD", 0).draw(meshes, GLMesh::PrimitiveType::TRIANGLES);
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}
void DepthAndTextureShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer,
		bool flatShading) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("MIN_DEPTH", camera.getNearPlane()).set("IS_FLAT",
			flatShading ? 1 : 0).set("MAX_DEPTH", camera.getFarPlane()).set(
			camera, frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}

EdgeDepthAndNormalShader::EdgeDepthAndNormalShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(	#version 330
				layout(location = 3) in vec3 vp0;
				layout(location = 4) in vec3 vp1;
				layout(location = 5) in vec3 vp2;
				layout(location = 6) in vec3 vp3;

													out VS_OUT {
					vec3 p0;
					vec3 p1;
					vec3 p2;
					vec3 p3;
				} vs_out;
				void main(void) {
					vs_out.p0=vp0;
					vs_out.p1=vp1;
					vs_out.p2=vp2;
					vs_out.p3=vp3;
				})",
			R"(	#version 330
				in vec3 v0, v1, v2, v3;
				in vec3 normal, vert;
				out vec4 FragColor;
				uniform float DISTANCE_TOL;
				//uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat; 
				uniform int IS_QUAD;
				
				vec3 slerp(vec3 p0, vec3 p1, float t){
				  p0=normalize(p0);
				  p1=normalize(p1);
				  float dotp = dot(p0,p1);
				  if ((dotp > 0.9999) || (dotp<-0.9999)){
					if (t<=0.5)return p0;
					return p1;
				  }
				  float theta = acos(dotp * 3.141596535/180.0);
				  return ((p0*sin((1-t)*theta) + p1*sin(t*theta)) / sin(theta));
				}

													void main(void) {
				  vec3 line, vec, proj;
				  float dists[4];
				  vec3 tan[4];
				  // compute minimum distance from current interpolated 3d vertex to triangle edges
				  // edge v1-v0
				  
				  vec = vert - v0;
				  line = normalize(v1 - v0);
				  proj = dot(vec, line) * line;
				  dists[0] = length (vec - proj);
				  tan[0]=cross(line,normal);
				  
				  vec = vert - v1;
                  line = normalize(v2 - v1);
				  proj = dot(vec, line) * line;
				  dists[1] = length (vec - proj);
				  tan[1]=cross(line,normal);

													if(IS_QUAD!=0){
                  vec = vert - v2;
				  line = normalize(v3 - v2); 
				  proj = dot(vec, line) * line;
				  dists[2] = length (vec - proj);
				  tan[2]=cross(line,normal);

													  line = normalize(v0 - v3); 
                  vec = vert - v3;
				  proj = dot(vec, line) * line;
				  dists[3] = length (vec - proj);
				  tan[3]=cross(line,normal);
				} else {
                  vec = vert - v2;
				  line = normalize(v0 - v2); 
				  proj = dot(vec, line) * line;
				  dists[2] = length (vec - proj);
				  tan[2]=cross(line,normal);
				}

													  vec3 outNorm=normalize(normal);
                  
                  float minDist=1E30;
                  for(int n=0;n<3+IS_QUAD;n++){
                     if(dists[n]<minDist){
				       outNorm=slerp(tan[n],normal,clamp(dists[n]/DISTANCE_TOL,0.0,1.0));
                       minDist=dists[n];
                     }
					}
				if (minDist <DISTANCE_TOL){
					FragColor = vec4(outNorm,minDist);
				} else {
					FragColor = vec4(outNorm,minDist);
				}
				})",
			R"(	#version 330
					layout (points) in;
					layout (triangle_strip, max_vertices=4) out;
					in VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
					} quad[];
					out vec3 v0, v1, v2, v3;
					out vec3 normal, vert;
					uniform int IS_QUAD;
				uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
					void main() {
					  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
					  mat4 VM=ViewModelMat*PoseMat;
					  
					  vec3 p0=quad[0].p0;
					  vec3 p1=quad[0].p1;
					  vec3 p2=quad[0].p2;
                      vec3 p3=quad[0].p3;
					
					  v0 = (VM*vec4(p0,1)).xyz;
					  v1 = (VM*vec4(p1,1)).xyz;
					  v2 = (VM*vec4(p2,1)).xyz;
                      v3 = (VM*vec4(p3,1)).xyz;
					  
					  
if(IS_QUAD!=0){
					  gl_Position=PVM*vec4(p0,1);  
					  vert = v0;
					  vec3 pt=0.25*(p0+p1+p2+p3);
        normal = cross(p0-pt, p1-pt)+cross(p1-pt, p2-pt)+cross(p2-pt, p3-pt)+cross(p3-pt, p0-pt);
		normal = (VM*vec4(normalize(-normal),0.0)).xyz;
														  EmitVertex();
} else {	  
					  gl_Position=PVM*vec4(p0,1);  
					  vert = v0;
					  normal = (VM*vec4(normalize(cross( p2-p0, p1-p0)),0.0)).xyz;
					  EmitVertex();
}
					  gl_Position=PVM*vec4(p1,1);  
					  vert = v1;
					  EmitVertex();

														if(IS_QUAD!=0){
					  gl_Position=PVM*vec4(p3,1);  
					  vert = v3;
					  EmitVertex();
					  gl_Position=PVM*vec4(p2,1);  
					   vert = v2;
					  EmitVertex();

														} else {
			          gl_Position=PVM*vec4(p2,1);  
					  vert = v2;
					  EmitVertex();
					}
					EndPrimitive();
	
					 })");
}
void EdgeDepthAndNormalShader::draw(
		const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("DISTANCE_TOL", camera.getScale()).set("IS_QUAD", 1).set(camera,
			frameBuffer.getViewport()).set("PoseMat", float4x4::identity()).draw(
			meshes, GLMesh::PrimitiveType::QUADS).end();
	begin().set("DISTANCE_TOL", camera.getScale()).set("IS_QUAD", 0).set(camera,
			frameBuffer.getViewport()).set("PoseMat", float4x4::identity()).draw(
			meshes, GLMesh::PrimitiveType::TRIANGLES).end();
	glEnable(GL_BLEND);
	frameBuffer.end();
}
void EdgeDepthAndNormalShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("DISTANCE_TOL", camera.getScale()).set(camera,
			frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}

void EdgeDepthAndNormalShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, GLFrameBuffer& frameBuffer) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	begin().set("DISTANCE_TOL", camera.getScale()).set(camera,
			frameBuffer.getViewport());
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();

	glEnable(GL_BLEND);
	frameBuffer.end();
}

EdgeEffectsShader::EdgeEffectsShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(
				#version 330
				layout(location = 0) in vec3 vp; 
				layout(location = 1) in vec2 vt; 
				uniform vec4 bounds;
				uniform vec4 viewport;
				out vec2 uv;
				void main() {
					uv=vt;
					vec2 pos=vp.xy*bounds.zw+bounds.xy;
					gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
				}
			)",
			R"(
				#version 330
				in vec2 uv;
				uniform ivec2 imageSize;
				uniform int KERNEL_SIZE;
				uniform sampler2D textureImage;
				uniform ivec2 depthBufferSize;
				const int SCALE=2;
				out vec4 FragColor;
				void main() {
				ivec2 pos=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
				vec4 rgba=texelFetch(textureImage, pos,0);//Do not interpolate depth buffer!
				vec4 nrgba;
				gl_FragDepth=rgba.w;
				if(rgba.w<1.0){
				float minDistance=KERNEL_SIZE*KERNEL_SIZE;
				for(int i=-KERNEL_SIZE;i<=KERNEL_SIZE;i++){
					for(int j=-KERNEL_SIZE;j<=KERNEL_SIZE;j++){
					  nrgba=texture(textureImage,uv+SCALE*vec2(i/float(imageSize.x),j/float(imageSize.y)));
					  if(nrgba.w>=1.0){
						  minDistance=min(minDistance,i*i+j*j);
					  }
					}
				}
				rgba=vec4(1.0-sqrt(minDistance)/KERNEL_SIZE,0.0,0.0,1.0);
				} else {
				float minDistance=KERNEL_SIZE*KERNEL_SIZE;
				for(int i=-KERNEL_SIZE;i<=KERNEL_SIZE;i++){
					for(int j=-KERNEL_SIZE;j<=KERNEL_SIZE;j++){
					  nrgba=texture(textureImage,uv+SCALE*vec2(i/float(imageSize.x),j/float(imageSize.y)));
					  if(nrgba.w<1.0){
						minDistance=min(minDistance,i*i+j*j);
					  }
				   }
				}
				rgba=vec4(0.0,1.0-sqrt(minDistance)/KERNEL_SIZE,0.0,1.0);
				} 
				FragColor=rgba;
				}
)");
}
OutlineShader::OutlineShader(
	bool onScreen , const std::shared_ptr<AlloyContext>& context) :
	GLShader(onScreen, context), innerGlowColor(0.5f, 0.1f,
		0.05f, 1.0f), outerGlowColor(0.1f, 0.05f, 0.5f, 1.0f), edgeColor(
			1.0f, 1.0f, 1.0f, 1.0f) {
	setLineWidth(2.0f);
	
	initialize({},
		R"(
			#version 330
			layout(location = 0) in vec3 vp; 
			layout(location = 1) in vec2 vt; 
			uniform vec4 bounds;
			uniform vec4 viewport;
			out vec2 uv;
			void main() {
				uv=vt;
				vec2 pos=vp.xy*bounds.zw+bounds.xy;
				gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
			}
		)",
		R"(
			#version 330
			in vec2 uv;
			uniform ivec2 imageSize;
			uniform int KERNEL_SIZE;
			uniform float LINE_WIDTH;
			uniform sampler2D textureImage;
			out vec4 FragColor;
			uniform vec4 innerColor,outerColor,edgeColor;
			void main() {
				float inside;
				ivec2 pos=ivec2(uv.x*imageSize.x,uv.y*imageSize.y);
				vec4 rgba=texelFetch(textureImage,pos,0);
				vec4 nrgba;
				float minDistance=KERNEL_SIZE*KERNEL_SIZE;
				gl_FragDepth=rgba.w;
				if(rgba.w<1.0){
					for(int i=-KERNEL_SIZE;i<=KERNEL_SIZE;i++){
						for(int j=-KERNEL_SIZE;j<=KERNEL_SIZE;j++){
							if(pos.x+i>=0&&pos.y+j>=0&&pos.x+i<imageSize.x&&pos.y+j<imageSize.y){
								nrgba=texelFetch(textureImage,pos+ivec2(i,j),0);
								if(nrgba.w>=1.0){
									minDistance=min(minDistance,i*i+j*j);
								}
							}
						}
					}
					inside=smoothstep(LINE_WIDTH,2*LINE_WIDTH,sqrt(minDistance)*2.0f);	
					rgba=mix(edgeColor,innerColor,inside);
				} else {
					minDistance=KERNEL_SIZE*KERNEL_SIZE;
					for(int i=-KERNEL_SIZE;i<=KERNEL_SIZE;i++){
						for(int j=-KERNEL_SIZE;j<=KERNEL_SIZE;j++){
							if(pos.x+i>=0&&pos.y+j>=0&&pos.x+i<imageSize.x&&pos.y+j<imageSize.y){
								nrgba=texelFetch(textureImage,pos+ivec2(i,j),0);
								if(nrgba.w<1.0){
									minDistance=min(minDistance,i*i+j*j);
								}
							}
						}
					}
					inside=smoothstep(LINE_WIDTH,2*LINE_WIDTH,sqrt(minDistance)*2.0f);	
					rgba=mix(edgeColor,outerColor,inside);
				}
				FragColor=rgba;
			}
)");

}
DistanceFieldShader::DistanceFieldShader(
	bool onScreen, const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context), kernelSize(8), innerGlowColor(0.5f, 0.1f,
				0.05f, 1.0f), outerGlowColor(0.1f, 0.05f, 0.5f,  1.0f), edgeColor(
				1.0f, 1.0f, 1.0f, 1.0f) {
	initialize( { },
			R"(
#version 330
layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
uniform vec4 bounds;
uniform vec4 viewport;

out vec2 uv;
void main() {
uv=vt;
vec2 pos=vp.xy*bounds.zw+bounds.xy;
gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
})",
			R"(
#version 330
in vec2 uv;
uniform ivec2 imageSize;
uniform int KERNEL_SIZE;
uniform sampler2D textureImage;
out vec4 FragColor;
uniform vec4 innerColor,outerColor,edgeColor;
float w=0;
void main() {
ivec2 pos=ivec2(uv.x*imageSize.x,uv.y*imageSize.y);
vec4 rgba=texelFetch(textureImage,pos,0);
vec4 nrgba;
float minDistance=KERNEL_SIZE*KERNEL_SIZE;
gl_FragDepth=rgba.w;
if(rgba.w<1.0){
for(int i=-KERNEL_SIZE;i<=KERNEL_SIZE;i++){
	for(int j=-KERNEL_SIZE;j<=KERNEL_SIZE;j++){
if(pos.x+i>=0&&pos.y+j>=0&&pos.x+i<imageSize.x&&pos.y+j<imageSize.y){
      nrgba=texelFetch(textureImage,pos+ivec2(i,j),0);
if(nrgba.w>=1.0){
      minDistance=min(minDistance,i*i+j*j);
}
}
	}
}
w=sqrt(minDistance)/KERNEL_SIZE;
rgba=mix(edgeColor,innerColor,w);
} else {
minDistance=KERNEL_SIZE*KERNEL_SIZE;
for(int i=-KERNEL_SIZE;i<=KERNEL_SIZE;i++){
	for(int j=-KERNEL_SIZE;j<=KERNEL_SIZE;j++){
      if(pos.x+i>=0&&pos.y+j>=0&&pos.x+i<imageSize.x&&pos.y+j<imageSize.y){
      nrgba=texelFetch(textureImage,pos+ivec2(i,j),0);
if(nrgba.w<1.0){
      minDistance=min(minDistance,i*i+j*j);
}
}
	}
}
w=sqrt(minDistance)/KERNEL_SIZE;
//if(w>0.99999)discard;
rgba=mix(edgeColor,outerColor,w);
}
FragColor=rgba;
})");
}

NormalColorShader::NormalColorShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(
#version 330
layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
uniform vec4 bounds;
uniform vec4 viewport;
out vec2 uv;
void main() {
uv=vt;
vec2 pos=vp.xy*bounds.zw+bounds.xy;
gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
})",
			R"(
#version 330
in vec2 uv;
const float PI=3.1415926535;
uniform sampler2D textureImage;
uniform ivec2 depthBufferSize;
out vec4 FragColor;
void main() {
ivec2 pos=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
vec4 rgba=texelFetch(textureImage, pos,0);//Do not interpolate depth buffer!
gl_FragDepth=rgba.w;
if(rgba.w<1.0){
float lum=clamp(abs(rgba.w),0.0,1.0);
rgba=vec4(-rgba.x*0.5+0.5,-rgba.y*0.5+0.5,-rgba.z,1.0);

} else {
rgba=vec4(0.0,0.0,0.0,1.0);
}
FragColor=rgba;
})");
}

DepthColorShader::DepthColorShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	initialize( { },
			R"(
#version 330
layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
uniform vec4 bounds;
uniform vec4 viewport;
out vec2 uv;
void main() {
uv=vt;
vec2 pos=vp.xy*bounds.zw+bounds.xy;
gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
})",
			R"(
#version 330
in vec2 uv;
const float PI=3.1415926535;
uniform sampler2D textureImage;
uniform float zMin;
uniform float zMax;
uniform ivec2 depthBufferSize;
out vec4 FragColor;
void main() {
ivec2 pos=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
vec4 rgba=texelFetch(textureImage, pos,0);//Do not interpolate depth buffer!
gl_FragDepth=rgba.w;
if(rgba.w<1.0){
	float lum=clamp((rgba.w-zMin)/(zMax-zMin),0.0,1.0);
	float r=max(0.0,1.0-lum*2.0);
	float b=min(2*lum-1.0,1.0);
	float g=max(0.0,1-2.0*abs(2.0*lum-1.0));
	rgba=vec4(r,g,b,1.0);
} else {
rgba=vec4(0.0,0.0,0.0,1.0);
}
FragColor=rgba;
})");
}

AmbientOcclusionShader::AmbientOcclusionShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context), sampleRadius(0.005f) {
	int thetaInc = 32;
	int phiInc = 8;
	for (int j = 1; j < phiInc; j++) {
		for (int i = 0; i < thetaInc; i++) {
			float sp = sin(0.5f * ALY_PI * (j) / (float) phiInc);
			float cp = cos(0.5f * ALY_PI * (j) / (float) phiInc);
			float3 v = float3(cos(2 * ALY_PI * i / (float) thetaInc) * cp,
					sin(2 * ALY_PI * i / (float) thetaInc) * cp, -sp);
			sampleNormals.push_back(v);
		}
	}
	initialize( { },
			R"(
#version 330

layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
uniform vec4 bounds;
uniform vec4 viewport;
out vec2 v_texCoord;
void main() {
v_texCoord=vt;
vec2 pos=vp.xy*bounds.zw+bounds.xy;
gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
})",
			MakeString()
					<< R"(
#version 330
in vec2 v_texCoord;
uniform sampler2D textureImage;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
uniform float u_radius;
uniform vec2 focalLength;

uniform ivec2 depthBufferSize;
float random(vec2 uv,float seed){
  vec2 v=vec2(12.9898,78.233);
  return fract(sin(dot(uv ,vec2(cos(seed)*v.x-sin(6.28*seed)*v.y,cos(6.28*seed)*v.y+sin(6.28*seed)*v.x))) * 43758.5453+seed);
}
float random2(vec2 uv,float seed){
  return random(vec2(random(uv,1.0-seed),random(uv,seed)),seed);
}
float toZ(float ndc){
	return -(ndc * (MAX_DEPTH - MIN_DEPTH) + MIN_DEPTH);
}
vec4 toWorld(vec2 uv,vec4 rgba){
	float z=toZ(rgba.w);
	return vec4(z*(2.0*uv.x-1)/focalLength.x,z*(2.0*uv.y-1)/focalLength.y, z, 1.0);	
}
ivec2 toCamera(vec4 pt){
	float z = pt.z;
	return ivec2(depthBufferSize.x*(0.5*pt.x*focalLength.x/z+0.5),depthBufferSize.y*(0.5*pt.y*focalLength.y/z+0.5));	
}
#define KERNEL_SIZE )"
					<< sampleNormals.size()
					<< R"(

uniform vec3 u_kernel[KERNEL_SIZE];
out vec4 FragColor;
void main(void)
{
   const float CAP_MIN_DISTANCE=0.001f;
   const float CAP_MAX_DISTANCE=0.005f;
	// Calculate out of the current fragment in screen space the view space position.
	float x = v_texCoord.x;
	float y = v_texCoord.y;
    ivec2 pos=ivec2(v_texCoord.x*depthBufferSize.x,v_texCoord.y*depthBufferSize.y);
	vec4 rgba=texelFetch(textureImage, pos,0);
	gl_FragDepth=rgba.w;
    if(length(rgba.xyz)==0.0){
		FragColor = vec4( 0,0,0,0);
        return; 
    }
	vec4 posProj = toWorld(v_texCoord,rgba);	
	vec3 normalView = rgba.xyz;
	vec3 randomVector = normalize(vec3(2*random2(v_texCoord,0.1367)-1,2*random2(v_texCoord,0.4632)-1,2*random2(v_texCoord,0.4632)-1));
	vec3 tangentView =normalize(randomVector - dot(randomVector, normalView) * normalView);
	
	vec3 bitangentView = cross(normalView, tangentView);
	mat3 kernelMatrix = mat3(tangentView, bitangentView, normalView); 
	float occlusion = 0.0;	
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		vec3 sampleVectorView = kernelMatrix * u_kernel[i];	
		vec4 samplePointNDC = posProj + u_radius * vec4(sampleVectorView, 0.0);
		ivec2 samplePointTexCoord = toCamera(samplePointNDC);   
		float zSceneNDC = toZ(texelFetch(textureImage, samplePointTexCoord,0).w);
		float delta = zSceneNDC-samplePointNDC.z;
		if (delta > CAP_MIN_DISTANCE && delta < CAP_MAX_DISTANCE){
			occlusion += 1.0;
		}
	}
	occlusion = 1.0 - occlusion / float(KERNEL_SIZE);
	FragColor = vec4(occlusion,occlusion,occlusion, 1.0);

									})");

}

PhongShader::PhongShader(int N, bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context) {
	lights.resize(N);
	initialize( { },
			R"(
#version 330
layout(location = 0) in vec3 vp; 
layout(location = 1) in vec2 vt; 
uniform vec4 bounds;
uniform vec4 viewport;
out vec2 uv;
void main() {
uv=vt;
vec2 pos=vp.xy*bounds.zw+bounds.xy;
gl_Position = vec4(2*pos.x/viewport.z-1.0,1.0-2*pos.y/viewport.w,0,1);
})",
			MakeString()
					<< R"(
#version 330
in vec2 uv;
const float PI=3.1415926535;
uniform sampler2D textureImage;
uniform vec2 focalLength;
uniform float MIN_DEPTH;
uniform float MAX_DEPTH;
uniform ivec2 depthBufferSize;
out vec4 FragColor;
const int MAX_LIGHTS=)"
					<< N << R"(;
uniform vec3 lightPositions[)" << N
					<< R"(];
uniform vec3 lightDirections[)" << N
					<< R"(];
uniform vec4 ambientColors[)" << N
					<< R"(];
uniform vec4 lambertianColors[)" << N
					<< R"(];
uniform vec4 diffuseColors[)" << N
					<< R"(];
uniform vec4 specularColors[)" << N
					<< R"(];
uniform float specularWeights[)" << N
					<< R"(];
float toZ(float ndc){
	return -(ndc * (MAX_DEPTH - MIN_DEPTH) + MIN_DEPTH);
}
vec4 toWorld(vec2 uv,vec4 rgba){
	float z=toZ(rgba.w);
	return vec4(z*(2.0*uv.x-1)/focalLength.x,z*(2.0*uv.y-1)/focalLength.y, z, 1.0);	
}
void main() {
   ivec2 pos=ivec2(uv.x*depthBufferSize.x,uv.y*depthBufferSize.y);
    vec4 rgba=texelFetch(textureImage, pos,0);//Do not interpolate depth buffer!
    if(rgba.w>=1.0)discard;
	vec4 pt=toWorld(uv,rgba);
	vec3 norm=rgba.xyz;
	vec4 outColor=vec4(0,0,0,0);
    float lsum=0.0;
	for(int i=0;i<MAX_LIGHTS;i++){
	  vec4 ambientColor=ambientColors[i];
	  vec4 diffuseColor=diffuseColors[i];
      vec4 lambertianColor=lambertianColors[i];
	  vec4 specularColor=specularColors[i];
	  float wsum=ambientColor.w+diffuseColor.w+specularColor.w+lambertianColor.w;
      lsum+=wsum;
	  if(wsum<=0.0)continue;
      vec3 specularDir = normalize(lightPositions[i] - pt.xyz);
	  vec3 viewDir = -normalize(pt.xyz);
	  vec3 reflectDir = reflect(-specularDir, norm);
	  float diffuse = max(dot(lightDirections[i],norm), 0.0);
      float lambert= max(dot(-specularDir,norm), 0.0);
	  float specular=0.0;
      if( specularWeights[i]>0.0&&specularColor.w>0.0){
	    specular = pow(max(dot(reflectDir, viewDir), 0.0), specularWeights[i]);
      }
	  outColor+=(   ambientColor.w*ambientColor
                  + diffuseColor.w*diffuse*diffuseColor
                  + lambertianColor.w* lambert * lambertianColor 
                  + specularColor.w* specular * specularColor);
	}
    outColor=outColor/lsum;
    outColor.w=1.0;
	FragColor=clamp(outColor,vec4(0.0,0.0,0.0,0.0),vec4(1.0,1.0,1.0,1.0));
	gl_FragDepth=rgba.w;
})");

}

WireframeShader::WireframeShader(bool onScreen,
		const std::shared_ptr<AlloyContext>& context) :
		GLShader(onScreen, context), lineWidth(2.0f), edgeColor(1.0f, 1.0f,
				1.0f, 1.0f), faceColor(0.0f, 0.1f, 0.0f, 0.0f),solid(true) {
	initialize({},
		R"(	#version 330
				layout(location = 3) in vec3 vp0;
				layout(location = 4) in vec3 vp1;
				layout(location = 5) in vec3 vp2;
				layout(location = 6) in vec3 vp3;
				out VS_OUT {
					vec3 p0;
					vec3 p1;
					vec3 p2;
					vec3 p3;
				} vs_out;
				void main() {
					vs_out.p0=vp0;
					vs_out.p1=vp1;
					vs_out.p2=vp2;
					vs_out.p3=vp3;
				})",
		R"(	#version 330
				in vec3 v0, v1, v2, v3;
				in vec3 normal, vert;
                in vec4 pos;
				uniform vec4 viewport;
				uniform float MIN_DEPTH;
				uniform float MAX_DEPTH;
				uniform vec4 edgeColor;
				uniform vec4 faceColor;
				uniform float LINE_WIDTH;
				uniform int IS_QUAD;
				uniform int IS_SOLID;
out vec4 FragColor;
				void main() {
				  vec3 line, vec, proj;
				  float dists[4];
				  vec = vert - v0;
				  line = normalize(v1 - v0);
				  proj = dot(vec, line) * line;
				  dists[0] = length ((vec - proj).xy*viewport.zw);
				  vec = vert - v1;
                  line = normalize(v2 - v1);
				  proj = dot(vec, line) * line;
				  dists[1] = length ((vec - proj).xy*viewport.zw);
				if(IS_QUAD!=0){
                  vec = vert - v2;
				  line = normalize(v3 - v2); 
				  proj = dot(vec, line) * line;
				  dists[2] = length ((vec - proj).xy*viewport.zw);
				  line = normalize(v0 - v3); 
                  vec = vert - v3;
				  proj = dot(vec, line) * line;
				  dists[3] = length ((vec - proj).xy*viewport.zw);
				} else {
                  vec = vert - v2;
				  line = normalize(v0 - v2); 
				  proj = dot(vec, line) * line;
				  dists[2] = length ((vec - proj).xy*viewport.zw);
				}
                  float minDist=1E30;
                  for(int n=0;n<3+IS_QUAD;n++){
                     if(dists[n]<minDist){
                       minDist=dists[n];
                     }
				  }
				float inside=smoothstep(LINE_WIDTH,2*LINE_WIDTH,minDist);	
				FragColor=mix(edgeColor,faceColor,inside);
				gl_FragDepth=(-pos.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
				if(IS_SOLID==0&&inside>0.5f){
					discard;
				}
		    })",
		R"(	#version 330
					layout (points) in;
					layout (triangle_strip, max_vertices=4) out;
					in VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
					} quad[];
					out vec3 v0, v1, v2, v3;
					out vec4 pos;
					out vec3 normal, vert;
					uniform int IS_QUAD;
				uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
					void main() {
					  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
					  mat4 VM=ViewModelMat*PoseMat;
					  vec4 q0,q1,q2,q3;
					  vec4 r0,r1,r2,r3;
					  vec3 p0=quad[0].p0;
					  vec3 p1=quad[0].p1;
					  vec3 p2=quad[0].p2;
                      vec3 p3=quad[0].p3;
					
					  r0=VM*vec4(p0,1);					  
                      r1=VM*vec4(p1,1);					  
                      r2=VM*vec4(p2,1);					  
                      r3=VM*vec4(p3,1);

					  q0=PVM*vec4(p0,1);					  
                      v0 = q0.xyz/q0.w;				  
					  q1=PVM*vec4(p1,1);					  
                      v1 = q1.xyz/q1.w;	
					  q2=PVM*vec4(p2,1);					  
                      v2 =q2.xyz/q2.w;		
					  q3=PVM*vec4(p3,1);					  
                      v3 = q3.xyz/q3.w;	
					  
					if(IS_QUAD!=0){
				      gl_Position=q0;					  
                      vert =v0;pos=r0;
					  vec3 pt=0.25*(p0+p1+p2+p3);
					  normal = cross(p0-pt, p1-pt)+cross(p1-pt, p2-pt)+cross(p2-pt, p3-pt)+cross(p3-pt, p0-pt);
					  normal = (VM*vec4(normalize(-normal),0.0)).xyz;
					  EmitVertex();
					} else {	  
				      gl_Position=q0;					  
                      vert =v0;pos=r0;		  
					  normal = (VM*vec4(normalize(cross( p2-p0, p1-p0)),0.0)).xyz;
					  EmitVertex();
					}
				      gl_Position=q1;					  
                      vert =v1;pos=r1;		  
					  EmitVertex();
					if(IS_QUAD!=0){
				      gl_Position=q3;					  
                      vert =v3;pos=r3;
					  EmitVertex();
				      gl_Position=q2;					  
                      vert =v2;pos=r2;
					  EmitVertex();
					} else {
				      gl_Position=q2;					  
                      vert =v2;pos=r2;
					  EmitVertex();
					}
					EndPrimitive();
	
					 })");
}
void WireframeShader::draw(Mesh& mesh, CameraParameters& camera,
		const GLFrameBuffer& frameBuffer) {
	frameBuffer.begin();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	draw( { &mesh }, camera, frameBuffer.getViewport());
	frameBuffer.end();
}
void WireframeShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const GLFrameBuffer& frameBuffer) {
	frameBuffer.begin();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	draw(meshes, camera, frameBuffer.getViewport());
	frameBuffer.end();
}
void WireframeShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const GLFrameBuffer& frameBuffer) {
	frameBuffer.begin();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	draw(meshes, camera, frameBuffer.getViewport());
	frameBuffer.end();
}
void WireframeShader::draw(const std::initializer_list<Mesh*>& meshes,
		CameraParameters& camera, const box2px& bounds) {
	begin().set("IS_SOLID",(solid?1:0)).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set(camera, bounds).set("viewport", bounds).set(
			"PoseMat", float4x4::identity()).set("LINE_WIDTH", lineWidth).set(
			"edgeColor", edgeColor).set("faceColor", faceColor).set("IS_QUAD",
			1).draw(meshes, GLMesh::PrimitiveType::QUADS).set("IS_QUAD", 0).draw(
			meshes, GLMesh::PrimitiveType::TRIANGLES).end();
}
void WireframeShader::draw(
		const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const box2px& bounds) {
	begin().set("IS_SOLID", (solid ? 1 : 0)).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("LINE_WIDTH", lineWidth).set("edgeColor",
			edgeColor).set("faceColor", faceColor).set(camera, bounds).set(
			"viewport", bounds);
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();
}
void WireframeShader::draw(
		const std::list<std::pair<Mesh*, float4x4>>& meshes,
		CameraParameters& camera, const box2px& bounds) {
	begin().set("IS_SOLID", (solid ? 1 : 0)).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
			camera.getFarPlane()).set("LINE_WIDTH", lineWidth).set("edgeColor",
			edgeColor).set("faceColor", faceColor).set(camera, bounds).set(
			"viewport", bounds);
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw( { pr.first },
					GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();
}


LineDistanceShader::LineDistanceShader(bool onScreen,
	const std::shared_ptr<AlloyContext>& context) :
	GLShader(onScreen, context), lineWidth(2.0f), solid(false) {
	initialize({},
		R"(	#version 330
				layout(location = 3) in vec3 vp0;
				layout(location = 4) in vec3 vp1;
				layout(location = 5) in vec3 vp2;
				layout(location = 6) in vec3 vp3;
				out VS_OUT {
					vec3 p0;
					vec3 p1;
					vec3 p2;
					vec3 p3;
				} vs_out;
				void main() {
					vs_out.p0=vp0;
					vs_out.p1=vp1;
					vs_out.p2=vp2;
					vs_out.p3=vp3;
				})",
		R"(	#version 330
				in vec3 v0, v1, v2, v3;
				in vec3 normal,vert;
				in vec4 pos;
				uniform vec4 viewport;
				uniform float MIN_DEPTH;
				uniform float MAX_DEPTH;
				uniform float LINE_WIDTH;
				uniform int IS_QUAD;
				uniform int IS_SOLID;
out vec4 FragColor;
				void main() {
				  vec3 line, vec, proj;
				  float dists[4];
				  vec = vert - v0;
				  line = normalize(v1 - v0);
				  proj = dot(vec, line) * line;
				  dists[0] = length ((vec - proj).xy*viewport.zw);
				  vec = vert - v1;
                  line = normalize(v2 - v1);
				  proj = dot(vec, line) * line;
				  dists[1] = length ((vec - proj).xy*viewport.zw);
				if(IS_QUAD!=0){
                  vec = vert - v2;
				  line = normalize(v3 - v2); 
				  proj = dot(vec, line) * line;
				  dists[2] = length ((vec - proj).xy*viewport.zw);
				  line = normalize(v0 - v3); 
                  vec = vert - v3;
				  proj = dot(vec, line) * line;
				  dists[3] = length ((vec - proj).xy*viewport.zw);
				} else {
                  vec = vert - v2;
				  line = normalize(v0 - v2); 
				  proj = dot(vec, line) * line;
				  dists[2] = length ((vec - proj).xy*viewport.zw);
				}
                  float minDist=1E30;
                  for(int n=0;n<3+IS_QUAD;n++){
                     if(dists[n]<minDist){
                       minDist=dists[n];
                     }
				  }
				float inside=smoothstep(LINE_WIDTH,2*LINE_WIDTH,minDist);	
				float d=(-pos.z-MIN_DEPTH)/(MAX_DEPTH-MIN_DEPTH);
				FragColor=vec4(normalize(normal),d);
				gl_FragDepth=d;
				if(IS_SOLID==0&&inside>0.5f){
					discard;
				}
		    })",
		R"(	#version 330
					layout (points) in;
					layout (triangle_strip, max_vertices=4) out;
					in VS_OUT {
						vec3 p0;
						vec3 p1;
						vec3 p2;
						vec3 p3;
					} quad[];
					out vec3 v0, v1, v2, v3;
					
					out vec3 normal, vert;
					out vec4 pos;
					uniform int IS_QUAD;
				uniform mat4 ProjMat, ViewMat, ModelMat,ViewModelMat,NormalMat,PoseMat; 
					void main() {
					  mat4 PVM=ProjMat*ViewModelMat*PoseMat;
					  mat4 VM=ViewModelMat*PoseMat;
					  vec4 q0,q1,q2,q3;
					  vec4 r0,r1,r2,r3;
					  vec3 p0=quad[0].p0;
					  vec3 p1=quad[0].p1;
					  vec3 p2=quad[0].p2;
                      vec3 p3=quad[0].p3;

					  r0=VM*vec4(p0,1);					  
                      r1=VM*vec4(p1,1);					  
                      r2=VM*vec4(p2,1);					  
                      r3=VM*vec4(p3,1);

					  q0=PVM*vec4(p0,1);					  
                      v0 = q0.xyz/q0.w;				  
					  q1=PVM*vec4(p1,1);					  
                      v1 = q1.xyz/q1.w;	
					  q2=PVM*vec4(p2,1);					  
                      v2 =q2.xyz/q2.w;		
					  q3=PVM*vec4(p3,1);					  
                      v3 = q3.xyz/q3.w;	
					  
					if(IS_QUAD!=0){
				      gl_Position=q0;					  
                      vert =v0;
					  pos = r0;
					  vec3 pt=0.25*(p0+p1+p2+p3);
					  normal = cross(p0-pt, p1-pt)+cross(p1-pt, p2-pt)+cross(p2-pt, p3-pt)+cross(p3-pt, p0-pt);
					  normal = (VM*vec4(normalize(-normal),0.0)).xyz;
					  EmitVertex();
					} else {	  
				      gl_Position=q0;					  
                      vert =v0;		
					  pos = r0;		  
					  normal = (VM*vec4(normalize(cross( p2-p0, p1-p0)),0.0)).xyz;
					  EmitVertex();
					}
				      gl_Position=q1;					  
                      vert =v1;		
					  pos = r1;		  
					  EmitVertex();
					if(IS_QUAD!=0){
				      gl_Position=q3;					  
                      vert= v3;
					  pos = r3;
					  EmitVertex();
				      gl_Position=q2;					  
                      vert= v2;
					  pos = r2;
					  EmitVertex();
					} else {
				      gl_Position=q2;					  
                      vert= v2;
					  pos = r2;
					  EmitVertex();
					}
					EndPrimitive();
	
					 })");
}
void LineDistanceShader::draw(Mesh& mesh, CameraParameters& camera,
	const GLFrameBuffer& frameBuffer) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	draw({ &mesh }, camera, frameBuffer.getViewport());
	frameBuffer.end();
}
void LineDistanceShader::draw(
	const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
	CameraParameters& camera, const GLFrameBuffer& frameBuffer) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	draw(meshes, camera, frameBuffer.getViewport());
	frameBuffer.end();
}
void LineDistanceShader::draw(
	const std::list<std::pair<Mesh*, float4x4>>& meshes,
	CameraParameters& camera, const GLFrameBuffer& frameBuffer) {
	frameBuffer.begin(float4(0.0f,0.0f,0.0f,1.0f));
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	draw(meshes, camera, frameBuffer.getViewport());
	frameBuffer.end();
}
void LineDistanceShader::draw(const std::initializer_list<Mesh*>& meshes,
	CameraParameters& camera, const box2px& bounds) {
	begin().set("IS_SOLID", (solid ? 1 : 0)).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
		camera.getFarPlane()).set(camera, bounds).set("viewport", bounds).set(
			"PoseMat", float4x4::identity()).set("LINE_WIDTH", lineWidth).set("IS_QUAD",
					1).draw(meshes, GLMesh::PrimitiveType::QUADS).set("IS_QUAD", 0).draw(
						meshes, GLMesh::PrimitiveType::TRIANGLES).end();
}
void LineDistanceShader::draw(
	const std::initializer_list<std::pair<Mesh*, float4x4>>& meshes,
	CameraParameters& camera, const box2px& bounds) {
	begin().set("IS_SOLID", (solid ? 1 : 0)).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
		camera.getFarPlane()).set("LINE_WIDTH", lineWidth).set(camera, bounds).set(
				"viewport", bounds);
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw({ pr.first },
				GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw({ pr.first },
				GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();
}
void LineDistanceShader::draw(
	const std::list<std::pair<Mesh*, float4x4>>& meshes,
	CameraParameters& camera, const box2px& bounds) {
	begin().set("IS_SOLID", (solid ? 1 : 0)).set("MIN_DEPTH", camera.getNearPlane()).set("MAX_DEPTH",
		camera.getFarPlane()).set("LINE_WIDTH", lineWidth).set(camera, bounds).set(
				"viewport", bounds);
	for (std::pair<Mesh*, float4x4> pr : meshes) {
		if (pr.first->quadIndexes.size() > 0) {
			set("IS_QUAD", 1).set("PoseMat", pr.second).draw({ pr.first },
				GLMesh::PrimitiveType::QUADS);
		}
		if (pr.first->triIndexes.size() > 0) {
			set("IS_QUAD", 0).set("PoseMat", pr.second).draw({ pr.first },
				GLMesh::PrimitiveType::TRIANGLES);
		}
	}
	end();
}
}

