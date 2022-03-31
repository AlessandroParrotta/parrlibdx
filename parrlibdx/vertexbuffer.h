#pragma once

#include <vector>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

namespace prb {
	class VertexBuffer {
	public:
		ID3D11Buffer* vptr = NULL;
		UINT stride = 0, offset = 0;
		UINT dataCapacity = 0; //bytes
		UINT vCount = 0; //vertex number

		void init(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* subRes);

		VertexBuffer();
		VertexBuffer(D3D11_BUFFER_DESC& desc, D3D11_SUBRESOURCE_DATA& subRes);
		VertexBuffer(void* data, size_t byteSize, UINT stride, UINT offset);
		VertexBuffer(std::vector<float> const& data, UINT stride, UINT offset);
		VertexBuffer(size_t byteSize, UINT stride, UINT offset);

		void resize(size_t size); //in bytes
		void setData(std::vector<float> const& data);

		void use();
		void release();

		void draw();

		void dispose();
	};

#ifndef PARRLIBDX_NAMESPACE_SHORTEN
	typedef VertexBuffer vbuf;
#endif
}
