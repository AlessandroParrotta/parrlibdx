#include "Texture.h"

#include <SOIL/SOIL.h>

#include "common.h"

#include "debug.h"
#include "util.h"

namespace prb {

    void Texture::defInit(unsigned char* data) {
        D3D11_TEXTURE2D_DESC tdesc;

        // ...
        // Fill out width, height, mip levels, format, etc...
        // ...

        tdesc.Width = width;
        tdesc.Height = height;
        tdesc.MipLevels = 1;
        tdesc.SampleDesc.Count = 1;		//how many textures
        tdesc.SampleDesc.Quality = 0;
        tdesc.ArraySize = 1;			//number of textures
        tdesc.Format = format;

        tdesc.Usage = D3D11_USAGE_DYNAMIC;
        tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // Add D3D11_BIND_RENDER_TARGET if you want to go
        // with the auto-generate mips route.
        tdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        tdesc.MiscFlags = 0; // or D3D11_RESOURCE_MISC_GENERATE_MIPS for auto-mip gen.

        D3D11_SUBRESOURCE_DATA srd; // (or an array of these if you have more than one mip level)
        srd.pSysMem = data; // This data should be in raw pixel format
        srd.SysMemPitch = linesize != 0 && linesize != width ? linesize : tdesc.Width * 4; // Sometimes pixel rows may be padded so this might not be as simple as width * pixel_size_in_bytes.
        srd.SysMemSlicePitch = 0; // tdesc.Width* tdesc.Height * 4;

        ThrowIfFailed(dev->CreateTexture2D(&tdesc, &srd, &texture));

        D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
        srDesc.Format = format;
        srDesc.ViewDimension = viewDimension; //D3D11_SRV_DIMENSION_TEXTURE2D;

        srDesc.Texture2D.MostDetailedMip = 0;
        srDesc.Texture2D.MostDetailedMip = 0;
        srDesc.Texture2D.MipLevels = 1;

        ThrowIfFailed(dev->CreateShaderResourceView(texture, &srDesc, &resView));
        //devcon->GenerateMips(resView);

        D3D11_SAMPLER_DESC samplerDesc;

        // Create a texture sampler state description.
        //samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        //samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.Filter = filtering;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0;
        samplerDesc.BorderColor[1] = 0;
        samplerDesc.BorderColor[2] = 0;
        samplerDesc.BorderColor[3] = 0;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        // Create the texture sampler state.
        ThrowIfFailed(dev->CreateSamplerState(&samplerDesc, &sampler));
    }

    void Texture::init(unsigned char* data, vec2 size) {
        this->width = size.x;
        this->height = size.y;

        defInit(data);
    }

    Texture::Texture() {}

    Texture::Texture(std::wstring const& path) {
        std::wstring ppath = strup::fallbackPath(path);
        this->path = ppath;

        unsigned char* data = SOIL_load_image(deb::toutf8(ppath).c_str(), &width, &height, &channels, SOIL_LOAD_RGBA);

        if (data) defInit(data);
        else deb::pr("could not load texture '", ppath, "'\n('", deb::toutf8(ppath).c_str(),"')\n");

        delete[] data;
    }
    Texture::Texture(std::string const& path) : Texture(deb::tos(path)) {}

    Texture::Texture(unsigned char* data, vec2 size) {
        this->width = size.x;
        this->height = size.y;

        defInit(data);
    }
    Texture::Texture(unsigned char* data, vec2 size, int linesize) {
        if (linesize == width) linesize *= 4;

        this->width = size.x;
        this->height = size.y;
        this->linesize = linesize;

        defInit(data);

        //TODO right now the texture is uploaded twice in this constructor

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ThrowIfFailed(devcon->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

        unsigned char* tdata = data;
        BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
        for (UINT i = 0; i < height; i++)
        {
            memcpy(mappedData, tdata, linesize);
            mappedData += mappedResource.RowPitch;
            tdata += linesize;
        }

        devcon->Unmap(texture, 0);
    }

    Texture::Texture(vec2 size) {
        this->linesize = size.x;
        unsigned char* sData = new unsigned char[size.x * size.y * 4];
        memset(sData, 0, size.x * size.y * 4);
        init(sData, size);
        delete[] sData;
    }

    Texture::Texture(vec2 size, int linesize, DXGI_FORMAT format, D3D_SRV_DIMENSION viewDimension) {
        if (linesize == (int)size.x) linesize *= 4;

        this->linesize = size.x;
        this->format = format;
        this->viewDimension = viewDimension;
        unsigned char* sData = new unsigned char[linesize * size.y];
        memset(sData, 0, linesize * size.y);
        init(sData, size);
        delete[] sData;
    }

    Texture::Texture(vec2 size, int linesize) : Texture(size, linesize, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_SRV_DIMENSION_TEXTURE2D) {}

    bool Texture::operator==(Texture const& ot)  const { 
        return this->texture == ot.texture && 
            this->resView == ot.resView && 
            this->sampler == ot.sampler &&

            this->width == ot.width && 
            this->height == ot.height &&
            this->channels == ot.channels &&
            this->linesize == ot.linesize;
    }
    bool Texture::operator!=(Texture const& ot)  const { return !(*this == ot); }

    bool Texture::null() const { return !texture || !resView || !sampler; }

    vec2 Texture::getSize() const { return { (float)width, (float)height }; }
    vec2 Texture::size() const { return getSize(); }

    void Texture::setData(unsigned char* data, vec2 size, int linesize) {
        if (linesize == width) linesize *= 4;

        this->width = size.x;
        this->height = size.y;
        this->linesize = linesize;

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ThrowIfFailed(devcon->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

        unsigned char* tdata = data;
        BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
        for (UINT i = 0; i < height; i++)
        {
            memcpy(mappedData, tdata, linesize);
            mappedData += mappedResource.RowPitch;
            tdata += linesize;
        }

        devcon->Unmap(texture, 0);
    }

    void Texture::setData(unsigned char* data, vec2 size) {
        this->width = size.x;
        this->height = size.y;

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ThrowIfFailed(devcon->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

        memcpy(mappedResource.pData, data, height * width * 4);

        devcon->Unmap(texture, 0);
    }

    void Texture::setData(vec4 data) {
        if (width <= 0 || height <= 0) return;

        //data.clamp(0.f, 1.f);
        
        unsigned char* ndata = new unsigned char[this->width * this->height * 4];

        for(int i=0; i< this->width * this->height * 4; i+=4){
            ndata[i + 0] = (unsigned char)(data.x*255.f);
            ndata[i + 1] = (unsigned char)(data.y*255.f);
            ndata[i + 2] = (unsigned char)(data.z*255.f);
            ndata[i + 3] = (unsigned char)(data.w*255.f);
        }

        setData(ndata, vec2(width, height));

        delete[] ndata;
    }

    D3D11_FILTER Texture::getFromFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag, TEXTURE_FILTERING mip) const {
        return prb::getFromFiltering(min, mag, mip);
    }

    void Texture::calcMinMagMip(D3D11_FILTER filter) {
        std::tuple<TEXTURE_FILTERING, TEXTURE_FILTERING, TEXTURE_FILTERING> ret = prb::calcMinMagMip(filter);
        min = std::get<0>(ret); mag = std::get<1>(ret); mip = std::get<2>(ret);
    }

    void Texture::setFiltering(D3D11_FILTER filter) { 
        this->filtering = filter;
        if (!texture || !resView || !sampler) return;

        calcMinMagMip(filter);

        if (sampler) { sampler->Release();  sampler = NULL; }

        D3D11_SAMPLER_DESC samplerDesc;
        samplerDesc.Filter = filtering;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0;
        samplerDesc.BorderColor[1] = 0;
        samplerDesc.BorderColor[2] = 0;
        samplerDesc.BorderColor[3] = 0;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        // Create the texture sampler state.
        ThrowIfFailed(dev->CreateSamplerState(&samplerDesc, &sampler));
    }
    void Texture::setFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag, TEXTURE_FILTERING mip) { this->min = min; this->mag = mag; this->mip = mip; setFiltering(getFromFiltering(min, mag, mip));  }
    void Texture::setFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag) { this->min = min; this->mag = mag; mip = mag; setFiltering(getFromFiltering(min, mag, mip)); }
    void Texture::setFiltering(TEXTURE_FILTERING filter) { min = mag = mip = filter; setFiltering(getFromFiltering(min, mag, mip)); }

    void Texture::drawImmediate(std::vector<float> const& verts) const {
        util::drawTexture(*this, verts);
    }

    void Texture::drawImmediate(vec2 pos, vec2 size) const {
        vec2 start = pos, end = pos + size;
        std::vector<float> verts = { //clockwise order
             start.x,start.y,       1.f,1.f,1.f,1.f,    0.f,1.f,
             start.x,end.y,         1.f,1.f,1.f,1.f,    0.f,0.f,
             end.x,end.y,           1.f,1.f,1.f,1.f,    1.f,0.f,

             end.x,end.y,           1.f,1.f,1.f,1.f,    1.f,0.f,
             end.x,start.y,         1.f,1.f,1.f,1.f,    1.f,1.f,
             start.x,start.y,       1.f,1.f,1.f,1.f,    0.f,1.f,
        };

        drawImmediate(verts);
    }

    void Texture::bind() const {
        devcon->PSSetShaderResources(0, 1, &resView);
        devcon->PSSetSamplers(0, 1, &sampler);
    }

    void Texture::dispose() {
        if(resView) resView->Release();
        if(texture) texture->Release();
        if(sampler) sampler->Release();
    }
}


///////////////////////////////////////
//OLD DRAWTEXTURE

        ////create a temporary vertex buffer
        //ID3D11Buffer* tempVBuffer = NULL;

        //// create the vertex buffer
        //D3D11_BUFFER_DESC bd;
        //ZeroMemory(&bd, sizeof(bd));

        //bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
        //bd.ByteWidth = sizeof(float) * verts.size();             // size is the VERTEX struct * 3
        //bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
        //bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

        //dev->CreateBuffer(&bd, NULL, &tempVBuffer);       // create the buffer


        //// copy the vertices into the buffer
        //D3D11_MAPPED_SUBRESOURCE ms;
        //devcon->Map(tempVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

        //memcpy(ms.pData, &verts[0], sizeof(float) * verts.size());

        //devcon->Unmap(tempVBuffer, NULL);


        //devcon->PSSetShaderResources(0, 1, &resView);
        //devcon->PSSetSamplers(0, 1, &sampler);

        //// select which vertex buffer to display
        //UINT stride = sizeof(float) * 2 + sizeof(float) * 4 + sizeof(float) * 2;
        //UINT offset = 0;
        //devcon->IASetVertexBuffers(0, 1, &tempVBuffer, &stride, &offset);

        //// select which primtive type we are using
        //devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //// draw the vertex buffer to the back buffer
        //devcon->Draw(6, 0);


        ////release the temporary buffer
        //tempVBuffer->Release();