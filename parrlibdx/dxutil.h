#pragma once

#include <vector>

#include <DirectXMath.h>

#include <parrlib/math/vector2f.h>
#include <parrlib/math/vector4f.h>
#include <parrlib/math/matrix3f.h>
#include <parrlib/math/matrix4f.h>

#include "shader.h"
#include "constantbuffer.h"

#ifndef NO_IMPL_CONCAT_STD_VECTOR_FLOAT
typedef std::vector<float> vertBatch;
inline std::vector<float> operator+(std::vector<float> const& a, std::vector<float> const& b) {
	std::vector<float> res = a; res.reserve(a.size() + b.size()); res.insert(res.end(), b.begin(), b.end()); return res;
}
inline void operator+=(std::vector<float>& a, std::vector<float> const& b) {
	a.insert(a.end(), b.begin(), b.end());
}
#endif

namespace prb {

	namespace DXUtil {

		extern bool useDefShader;
		void setDXMat3x3(DirectX::XMFLOAT3X3& mat, mat3 const& m);
		DirectX::XMFLOAT3X3 getDXMat3x3(mat3 const& m);

		void setDXMat4x4(DirectX::XMFLOAT4X4& mat, mat4 const& m);
		DirectX::XMFLOAT4X4 getDXMat4x4(mat4 const& m);

		mat4 toMat4(mat3 const& mat);

		struct CBStruct {
			mat4 mat;
		};
		extern CBStruct cbstrc;
		extern cbuf defCB;
		extern Shader uSh;

		void init();

		extern std::vector<mat3> mStack; //matrix stack
		void pushMatrix();
		void pushMatrix(const mat3& modifier);
		void pushReset();
		void popMatrix();
		mat3 getTopMatrix();
		mat3 getTopMatrixAspectInv();
		mat3 getTopMatrixInvAspect();
		mat3 getTopMatrixOrthoInverted();

		mat3 getAspectOrthoX();
		mat3 getAspectOrthoY();
		mat3 getMinAspectOrtho();
		mat3 getMaxAspectOrtho();
		mat3 getAspectOrtho(); 
		
		mat3 getAspectOrthoXInv();
		mat3 getMinAspectOrthoInv();
		mat3 getMaxAspectOrthoInv();
		mat3 getAspectOrthoInv();

		aabb2 getScreenBoundingBox();

		mat3 fromNdc();
		mat3 toNdc();

		mat3 fromNdcAspect();
		mat3 toNdcAspect();

		mat3 fromNdcAspectInv();
		mat3 toNdcAspectInv();

		aabb2 getResbbNDCAspect();

		void translate(vec2 v);
		void rotate(float angle);
		void scale(vec2 v);
		void ortho(float left, float right, float bottom, float top);
		void lookAt(vec2 eye, vec2 pos);
		void multMatrix(const mat3& m);
		void resetMatrix();

		mat3 getQuadrantMatrix(int idx, int nPerRow);



		struct utilDrawState {
			bool outline;
			float strokeWidth;
			vec4 strokeColor = vc4::black;
			vec4 color = vc4::black;

			bool batching = true;
		};
		extern utilDrawState drawState;

		void setColor(vec4 color);
		vec4 getColor();

		void batchTris(std::vector<float>& res, std::vector<float> const& verts);
		void batchShapeConvex(std::vector<float>& res, std::vector<vec2> const& verts);
		void batchShape(std::vector<float>& res, std::vector<vec2> const& verts);
		void batchQuad(std::vector<float>& res, aabb2 quad, const mat3 mat = 1.f);
		//void batchTexture(std::vector<float>& res, const mat3 mat = 1.f);
		void batchCircle(std::vector<float>& res, mat3 mat, const int detail = 40);
		void batchFancyLine(std::vector<float>& res, const std::vector<vec2>& ps, float width, bool loop, const int& style = 0);

		std::vector<float> batchTris(std::vector<float> verts);
		std::vector<float> batchTris(std::vector<vec2> const& verts);
		std::vector<float> batchShapeConvex(std::vector<vec2> const& verts);
		std::vector<float> batchShape(std::vector<vec2> const& verts);
		std::vector<float> batchQuad(aabb2 quad, const mat3 mat = 1.f);
		//std::vector<float> batchTexture(const mat3 mat = 1.f);
		std::vector<float> batchCircle(mat3 mat, const int detail = 40);
		std::vector<float> batchFancyLine(const std::vector<vec2>& ps, float width, bool loop, const int& style = 0);

		void drawTrisNoMatStack(std::vector<float> const& verts);
		void drawTris(std::vector<float> const& verts);
		void drawTris(std::vector<vec2> const& verts);
		void drawShapeConvex(std::vector<vec2> const& verts);
		void drawShape(std::vector<vec2> const& verts);
		void drawShape(std::vector<vec2> verts, mat3 mat);
		void drawPoly(std::vector<vec2> verts);
		void drawQuad(vec2 start, vec2 size, const mat3 mat = 1.f);
		void drawQuad(aabb2 quad, const mat3 mat = 1.f);
		void drawCircle(mat3 mat, const int detail = 40);

		// vertex layout should be { x,y, colorR, colorG, colorB, colorA }
		void drawFancyLine(const std::vector<vec2>& ps, float width, bool loop, const int& style = 0);

		void drawTexture(std::vector<float> const& verts);
		void drawTexture(Texture const& t, std::vector<float> const& verts);
		void drawTexture(std::wstring const& path, std::vector<float> const& verts);

		void drawTexture(const mat3 mat, vec2 const& uv1, vec2 const& uv2, vec2 const& uv3, vec2 const& uv4);
		void drawTexture(const mat3 mat, aabb2 const& uv);
		void drawTexture(const mat3 mat = 1.f);

		void drawTexture(std::wstring const& path, const mat3 mat, vec2 const& uv1, vec2 const& uv2, vec2 const& uv3, vec2 const& uv4);
		void drawTexture(std::wstring const& path, const mat3 mat, aabb2 const& uv);
		void drawTexture(std::wstring const& path, const mat3 mat = 1.f);

		void bindTexture(ID3D11ShaderResourceView* resView, ID3D11SamplerState* sampler);
		void drawTexture(ID3D11ShaderResourceView* resView, ID3D11SamplerState* sampler, const mat3 mat = 1.f);
		void drawTexture(Texture const& t, const mat3 mat = 1.f);
	}

	namespace dxutil = DXUtil;
}
