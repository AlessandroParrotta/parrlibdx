#pragma once

#include "Texture.h"

namespace prb {
	class FlowTexture : public Texture {
	public:

		unsigned char* data = NULL;

		void init(unsigned char* data, vec2 size);

		FlowTexture();
		FlowTexture(std::wstring const& path);
		FlowTexture(std::string const& path);
		FlowTexture(unsigned char* data, vec2 size);
		FlowTexture(unsigned char* data, vec2 size, int linesize);
		FlowTexture(vec2 size);
		FlowTexture(vec2 size, int linesize, DXGI_FORMAT format, D3D_SRV_DIMENSION viewDimension);
		FlowTexture(vec2 size, int linesize);

		void fillRegion(unsigned char* subData, vec2 start, vec2 end);
		std::vector<unsigned char> getData();

		void resize(vec2 newSize);

		void setData(unsigned char* data, vec2 size, int linesize);
		void setData(unsigned char* data, vec2 size);
		void setData(vec4 data);

		void dispose();
	};
}
