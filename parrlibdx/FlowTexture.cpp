#include "FlowTexture.h"

#include <SOIL/SOIL.h>

#include "debug.h"
#include "util.h"

namespace prb {

	void FlowTexture::init(unsigned char* data, vec2 size) {
		this->width = size.x;
		this->height = size.y;

		unsigned char* cpData = new unsigned char[width * height * 4];
		memcpy(cpData, data, (width * height * 4));

		this->data = cpData;

		defInit(cpData);
	}

	FlowTexture::FlowTexture() : Texture() {}
	FlowTexture::FlowTexture(std::wstring const& path){
		this->path = path;

		unsigned char* data = SOIL_load_image(deb::toutf8(path).c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		
		unsigned char* cpData = new unsigned char[width * height * 4];
		memcpy(cpData, data, (width * height * 4));
		this->data = cpData;

		if (data != NULL) defInit(data);

		delete[] data;
	}
	FlowTexture::FlowTexture(std::string const& path) : FlowTexture(deb::tos(path)) {}
	FlowTexture::FlowTexture(unsigned char* data, vec2 size) : Texture(data, size) {
		unsigned char* cpData = new unsigned char[width * height * 4];
		memcpy(cpData, data, (width * height * 4));
		this->data = cpData;
	}
	FlowTexture::FlowTexture(unsigned char* data, vec2 size, int linesize) : Texture(data, size, linesize) {
		unsigned char* cpData = new unsigned char[linesize * height];
		memcpy(cpData, data, (linesize * height));
		this->data = cpData;
	}
	FlowTexture::FlowTexture(vec2 size) {
		this->linesize = size.x;
		unsigned char* sData = new unsigned char[size.x * size.y * 4];
		memset(sData, 0, size.x * size.y * 4);
		init(sData, size);
		delete[] sData;
	}
	FlowTexture::FlowTexture(vec2 size, int linesize, DXGI_FORMAT format, D3D_SRV_DIMENSION viewDimension) {
		if (linesize == (int)size.x) linesize *= 4;

		this->linesize = size.x;
		this->format = format;
		this->viewDimension = viewDimension;
		unsigned char* sData = new unsigned char[linesize * size.y];
		memset(sData, 0, linesize * size.y);
		init(sData, size);
		delete[] sData;
	}
	FlowTexture::FlowTexture(vec2 size, int linesize) : FlowTexture(size, linesize, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_SRV_DIMENSION_TEXTURE2D) {}


	//size must be equal to the size of subData
	void FlowTexture::fillRegion(unsigned char* subData, vec2 start, vec2 size) {
		//deb::out("fillregion ", start, " -> ", start + size, " (", size, ")\n");

		//vec2 end = start + size;

		//if (start.x < 0.f) start.x = 0.f;
		//if (start.x > getSize().x) start.x = getSize().x;

		//if (start.y < 0.f) start.y = 0.f;
		//if (start.y > getSize().y) start.y = getSize().x;

		//if (end.x < 0.f) end.x = 0.f;
		//if (end.x > getSize().x) end.x = getSize().y;

		//if (end.y < 0.f) end.y = 0.f;
		//if (end.y > getSize().y) end.y = getSize().y;

		////deb::out("size: ", getSize(), ", region size: ", size, "\n");
		////deb::out("start: ", start, ", end: ", end, "\n");

		//for (int y = start.y; y < end.y; y++) {
		//	//deb::out("copying row ", y, ", ", start.x, " -> ", end.x, " (0 -> ", size.x, ", ", start.x, " -> ", end.x, ")\n");
		//	for (int x = start.x; x < end.x; x++) {
		//		int dstidx = ((int)y * (int)getSize().x + (int)x)*4;
		//		int srcidx = (((int)y-(int)start.y) * (int)size.x + ((int)x-(int)start.x))*4;

		//		//if (dstidx > getSize().x * getSize().y * 4) deb::out("dstidx outside bounds: ", dstidx, "\n");
		//		//if (dstidx < 0) deb::out("dstidx outside bounds: ", dstidx, "\n");
		//		
		//		//if (srcidx > size.x * size.y * 4) deb::out("srcidx outside bounds: ", dstidx, "\n");
		//		//if (srcidx < 0) deb::out("srcidx outside bounds: ", dstidx, "\n");

		//		//deb::out("copying ", srcidx + 0, ", ", srcidx + 1, ", ", srcidx + 2, ", ", srcidx + 3, "\n");
		//		//deb::out("into ", dstidx + 0, ", ", dstidx + 1, ", ", dstidx + 2, ", ", dstidx + 3, "\n");
		//		//deb::out("limit: ", getSize().x * getSize().y * 4, ", ", size.x * size.y * 4, "\n");
		//		
		//		data[dstidx + 0] = subData[srcidx + 0];
		//		data[dstidx + 1] = subData[srcidx + 1];
		//		data[dstidx + 2] = subData[srcidx + 2];
		//		data[dstidx + 3] = subData[srcidx + 3];
		//	}
		//}

		
		//deb::out("fillRegion\n");
		start = start.minned(getSize()).maxed(0.f);
		vec2 end = (start + size).minned(getSize()).maxed(0.f);
		vec2 oldSize = size;
		size = (end - start).minned(oldSize);

		//deb::out(start, " -> ", end, "\n");

		int width = size.x;
		int height = size.y;

		for (int y = (int)start.y; y < (int)end.y; y++) {
			int srcIdx = (y - (int)start.y) * width + 0;
			int dstIdx = y * this->width + (int)start.x;
			memcpy(&(data[dstIdx * 4]), &(subData[srcIdx * 4]), (width * 4));
			//deb::out("memcpy ", srcIdx * 4, " -> ", srcIdx * 4 + width*4, " (", width*height*4, ") ", dstIdx * 4, " -> ", dstIdx * 4 +width*4," (", this->width*this->height*4, ") (", start.xf(y), " -> ", (start+size).xf(y), ") ", width * 4, " (", getSize(), ")\n");
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ThrowIfFailed(devcon->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		//deb::out("mapped\n");

		unsigned char* tdata = data;
		BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
		BYTE* startx = mappedData;
		//deb::out("rowpitch width ", mappedResource.RowPitch, " ", this->width * 4, "\n");
		for (UINT i = 0; i < this->height; i++)
		{
			memcpy(mappedData, tdata, this->width * 4);
			mappedData += mappedResource.RowPitch;
			tdata += this->width * 4;
		}
		//deb::out(tdata - this->width*4, "\n");

		devcon->Unmap(texture, 0);

		//deb::out("unmapped\n");
	}

	std::vector<unsigned char> FlowTexture::getData() {
		std::vector<unsigned char> data; data.resize(width * height * 4);
		memcpy(&(data[0]), this->data, (width * height * 4));
		return data;
	}

	void FlowTexture::resize(vec2 newSize) {
		FlowTexture t(newSize);
		//deb::out("resize ", getSize(), " -> ", newSize, "\n");
		t.fillRegion(data, 0.f, getSize());
		dispose();
		*this = t;
	}

	void FlowTexture::setData(unsigned char* data, vec2 size, int linesize) {
		Texture::setData(data, size, linesize);

		if (this->data) delete[] this->data;
		unsigned char* cpData = new unsigned char[linesize * size.y];
		memcpy(cpData, data, linesize * size.y);

		this->data = cpData;
	}

	void FlowTexture::setData(unsigned char* data, vec2 size) {
		Texture::setData(data, size);

		if (this->data) delete[] this->data;

		unsigned char* cpData = new unsigned char[size.x * size.y * 4];
		memcpy(cpData, data, size.x * size.y * 4);

		this->data = cpData;
	}

	void FlowTexture::setData(vec4 data) {
		if (width <= 0 || height <= 0) return;

	}

	void FlowTexture::dispose() {
		Texture::dispose();

		delete[] data;
	}
}

//////////////////////////////////////
//OTHER GETDATA

		//std::vector<unsigned char> data; data.resize(height * linesize);
		//D3D11_MAPPED_SUBRESOURCE mappedResource;
		//ThrowIfFailed(devcon->Map(texture, 0, D3D11_MAP_READ, 0, &mappedResource));

		//BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
		//for (int y = 0; y < height; y++) {
		//	for (int x = 0; x < linesize; x++) {
		//		int i = y * linesize + x;
		//		data[i] = mappedData[i];
		//	}
		//}

		//devcon->Unmap(texture, 0);
		//return data;
