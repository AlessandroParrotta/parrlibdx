#pragma once

#include <string>
#include <vector>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

#include <parrlib/math/vector2f.h>

namespace prb {

	class Texture {
	protected:
		void defInit(unsigned char* data);
		void init(unsigned char* data, vec2 size);

	public:
		ID3D11Texture2D* texture = NULL;
		ID3D11ShaderResourceView* resView = NULL;
		ID3D11SamplerState* sampler = NULL;

		D3D_SRV_DIMENSION viewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

		std::wstring path = L"";

		int width = 0, height = 0, channels = 0, linesize = 0;

		Texture();
		Texture(std::wstring const& path);
		Texture(std::string const& path);
		Texture(unsigned char* data, vec2 size);
		Texture(unsigned char* data, vec2 size, int linesize);
		Texture(vec2 size);
		Texture(vec2 size, int linesize, DXGI_FORMAT format, D3D_SRV_DIMENSION viewDimension);
		Texture(vec2 size, int linesize);

		vec2 getSize();

		void setData(unsigned char* data, vec2 size, int linesize);
		void setData(unsigned char* data, vec2 size);

		//extremely inefficient, DO NOT USE unless it's for testing
		//this assumes there is already a shader bound to the device
		//vertex layout should be {posX, posY, colorR, colorG, colorB, colorA, uvX, uvY}
		void drawImmediate(std::vector<float> const& verts);

		//extremely inefficient, DO NOT USE unless it's for testing
		//this assumes there is already a shader bound to the device
		void drawImmediate(vec2 pos, vec2 size);

		void bind();

		void dispose();
	};

}
