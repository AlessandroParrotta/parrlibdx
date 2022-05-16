#pragma once

#include <string>
#include <vector>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

#include <parrlibcore/vector2f.h>
#include <parrlibcore/vector4f.h>
#include <parrlibcore/matrix3f.h>

#include "common.h"

namespace prb {
	class FrameBuffer {
	public:
		ID3D11Texture2D* texture = NULL;
		ID3D11ShaderResourceView* resView = NULL;
		ID3D11SamplerState* sampler = NULL;

		ID3D11Texture2D* depthStencilTex = NULL;
		ID3D11DepthStencilState* depthStencilState = NULL;
		ID3D11DepthStencilView* depthStencilView = NULL;

		ID3D11RenderTargetView* rtv;

		D3D11_VIEWPORT viewport;

		vec2 res;
		vec2 oldres;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//UINT sampleCount = 8; //AntiAliasing
		//D3D_SRV_DIMENSION viewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		//D3D11_RTV_DIMENSION rtvDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		UINT sampleCount = 1; //AntiAliasing
		D3D_SRV_DIMENSION viewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		D3D11_RTV_DIMENSION rtvDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		TEXTURE_FILTERING min = LINEAR, mag = LINEAR, mip = LINEAR;
		D3D11_FILTER filtering = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

		void defInit();

		FrameBuffer();
		FrameBuffer(vec2 res, DXGI_FORMAT format, UINT sampleCount, D3D_SRV_DIMENSION viewDimension, D3D11_RTV_DIMENSION rtvDimension);
		FrameBuffer(vec2 res, DXGI_FORMAT format);
		FrameBuffer(vec2 res);

		void resize(vec2 res);

		vec2 size() const;

		void clear(vec4 color);
		void clear();

		void bind();
		void unbind();

		D3D11_FILTER getFromFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag, TEXTURE_FILTERING mip) const;
		void calcMinMagMip(D3D11_FILTER filter);

		void setFiltering(D3D11_FILTER filter);
		void setFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag, TEXTURE_FILTERING mip);
		void setFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag);
		void setFiltering(TEXTURE_FILTERING filter);

		void bindTex();
		void drawImmediate(mat3 mat);

		void dispose();
	};

	typedef FrameBuffer fbuf;
}
