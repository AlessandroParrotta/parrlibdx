#include "vertexbuffer.h"

#include "common.h"
#include "debug.h"

namespace prb {
	void VertexBuffer::init(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* subRes) {
		ThrowIfFailed(dev->CreateBuffer(desc, subRes, &vptr));
	}

	VertexBuffer::VertexBuffer() {}
	VertexBuffer::VertexBuffer(D3D11_BUFFER_DESC& desc, D3D11_SUBRESOURCE_DATA& subRes) { init(&desc, &subRes); }
	VertexBuffer::VertexBuffer(void* data, size_t byteSize, UINT stride, UINT offset) {
		this->stride = stride;
		this->offset = offset;

		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = byteSize;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subRes;
		subRes.pSysMem = data;
		subRes.SysMemPitch = 0;
		subRes.SysMemSlicePitch = 0;

		dataCapacity = byteSize;
		vCount = dataCapacity / stride;

		init(&desc, &subRes);
	}

	VertexBuffer::VertexBuffer(std::vector<float> const& data, UINT stride, UINT offset) {
		this->stride = stride;
		this->offset = offset;

		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(float) * data.size();
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subRes;
		subRes.pSysMem = &data[0];
		subRes.SysMemPitch = 0;
		subRes.SysMemSlicePitch = 0;

		dataCapacity = data.size() * sizeof(float);
		vCount = dataCapacity / stride;

		//ThrowIfFailed(dev->CreateBuffer(&desc, NULL, &vptr));
		init(&desc, &subRes);
	}

	VertexBuffer::VertexBuffer(size_t byteSize, UINT stride, UINT offset) {
		this->stride = stride;
		this->offset = offset;

		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = byteSize;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		dataCapacity = byteSize;
		vCount = dataCapacity / stride;

		init(&desc, NULL);
	}

	void VertexBuffer::resize(size_t size) {
		//deb::ss << "Resize!! " << size << " " << curtime << "\n";
		vbuf vb(size, stride, offset);
		dispose();
		*this = vb;
	}

	void VertexBuffer::setData(std::vector<float> const& data) {
		if (data.size() <= 0) return;

		if (data.size() * sizeof(float) > dataCapacity) resize(data.size() * sizeof(float));
		else if (data.size() * sizeof(float) < dataCapacity / 4) resize(data.size() * sizeof(float));
		else vCount = data.size() / (stride / sizeof(float));

		D3D11_MAPPED_SUBRESOURCE ms;
		devcon->Map(vptr, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

		memcpy(ms.pData, &data[0], sizeof(float) * data.size());

		devcon->Unmap(vptr, NULL);
	}

	void VertexBuffer::use() {
		devcon->IASetVertexBuffers(0, 1, &vptr, &stride, &offset);
		devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void VertexBuffer::release() {
		//devcon->IASetVertexBuffers(0, 1, 0, &stride, &offset);
	}

	void VertexBuffer::draw() {
		use();
		devcon->Draw(vCount, 0);
	}

	void VertexBuffer::dispose() {
		vptr->Release();
	}
}
