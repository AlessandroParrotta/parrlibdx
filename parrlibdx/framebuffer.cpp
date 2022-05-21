#include "framebuffer.h"

#include <parrlibcore/constants.h>

#include "common.h"
//#include "vertexbuffer.h"
#include "shader.h"

#include "debug.h"
#include "util.h"

#include "context.h"

namespace prb {

    void FrameBuffer::defInit() {
        res = res.maxed(1.f);

        D3D11_TEXTURE2D_DESC texDesc;
        ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));

        texDesc.Width = res.x;
        texDesc.Height = res.y;
        texDesc.Format = format;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.SampleDesc.Count = sampleCount;
        texDesc.SampleDesc.Quality = 0;
        texDesc.CPUAccessFlags = 0;
        texDesc.ArraySize = 1;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.MiscFlags = 0;
        texDesc.MipLevels = 1;

        //tdesc.Usage = D3D11_USAGE_DEFAULT;
        //tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // Add D3D11_BIND_RENDER_TARGET if you want to go
        //// with the auto-generate mips route.
        //tdesc.CPUAccessFlags = 0;
        //tdesc.MiscFlags = 0; // or D3D11_RESOURCE_MISC_GENERATE_MIPS for auto-mip gen.

        ThrowIfFailed(dev->CreateTexture2D(&texDesc, NULL, &texture));

        D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
        srDesc.Format = texDesc.Format;
        srDesc.ViewDimension = viewDimension; //D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;//D3D11_SRV_DIMENSION_TEXTURE2D;
        srDesc.Texture2D.MostDetailedMip = 0;
        srDesc.Texture2D.MipLevels = 1;

        ThrowIfFailed(dev->CreateShaderResourceView(texture, &srDesc, &resView));

        D3D11_SAMPLER_DESC samplerDesc;

        // Create a texture sampler state description.
        //samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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

        //depth stencil texture 
        D3D11_TEXTURE2D_DESC dptexDesc;
        ZeroMemory(&dptexDesc, sizeof(D3D11_TEXTURE2D_DESC));

        dptexDesc.Width = res.x;
        dptexDesc.Height = res.y;
        dptexDesc.MipLevels = 1;
        dptexDesc.ArraySize = 1;
        dptexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dptexDesc.SampleDesc.Count = sampleCount;
        dptexDesc.SampleDesc.Quality = 0;
        dptexDesc.Usage = D3D11_USAGE_DEFAULT;
        dptexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        dptexDesc.CPUAccessFlags = 0;
        dptexDesc.MiscFlags = 0;

        ThrowIfFailed(dev->CreateTexture2D(&dptexDesc, NULL, &depthStencilTex));
        ThrowIfFailed(dev->CreateDepthStencilView(depthStencilTex, NULL, &depthStencilView));


        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.StencilReadMask = 0xFF;
        depthStencilDesc.StencilWriteMask = 0xFF;
        depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        ThrowIfFailed(dev->CreateDepthStencilState(&depthStencilDesc, &depthStencilState));

        //render target
        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
        renderTargetViewDesc.Format = texDesc.Format;

        //renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;//D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.ViewDimension = rtvDimension;//D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;

        ThrowIfFailed(dev->CreateRenderTargetView(texture, &renderTargetViewDesc, &rtv));

        ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = res.x;
        viewport.Height = res.y;
    }

    FrameBuffer::FrameBuffer() {}
    FrameBuffer::FrameBuffer(vec2 res, DXGI_FORMAT format, UINT sampleCount, D3D_SRV_DIMENSION viewDimension, D3D11_RTV_DIMENSION rtvDimension) {
        this->res = res;
        this->format = format;
        this->sampleCount = sampleCount;
        this->viewDimension = viewDimension;
        this->rtvDimension = rtvDimension;
        defInit();
    }

    FrameBuffer::FrameBuffer(vec2 res, DXGI_FORMAT format) {
        this->res = res;
        this->format = format;
        defInit();
    }

    FrameBuffer::FrameBuffer(vec2 res) : FrameBuffer(res, DXGI_FORMAT_B8G8R8A8_UNORM) {}

    void FrameBuffer::resize(vec2 res) {
        dispose();
        FrameBuffer fb(res, format, sampleCount, viewDimension, rtvDimension);
        fb.setFiltering(filtering);
        *this = fb;
    }

    vec2 FrameBuffer::size() const { return res; }

    void FrameBuffer::clear(vec4 color) {
        float tcol[4] = { color.x, color.y, color.z, color.w };

        // Clear the back buffer.
        devcon->ClearRenderTargetView(rtv, tcol);

        // Clear the depth buffer.
        devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
    void FrameBuffer::clear() { clear(vec4(0.f, 0.f, 0.f, 1.f)); }

    void FrameBuffer::bind() {
        devcon->OMSetRenderTargets(1, &rtv, depthStencilView);
        devcon->OMSetDepthStencilState(depthStencilState, 0);
        devcon->RSSetViewports(1, &viewport);

        oldres = cst::res();
        cst::res(res);
    }

    void FrameBuffer::unbind() {
        devcon->OMSetRenderTargets(1, &backbuffer, NULL);

        cst::res(oldres);

        D3D11_VIEWPORT viewport;
        ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = cst::resx();
        viewport.Height = cst::resy();

        devcon->RSSetViewports(1, &viewport);
    }

    D3D11_FILTER FrameBuffer::getFromFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag, TEXTURE_FILTERING mip) const {
        return prb::getFromFiltering(min, mag, mip);
    }

    void FrameBuffer::calcMinMagMip(D3D11_FILTER filter) {
        std::tuple<TEXTURE_FILTERING, TEXTURE_FILTERING, TEXTURE_FILTERING> ret = prb::calcMinMagMip(filter);
        min = std::get<0>(ret); mag = std::get<1>(ret); mip = std::get<2>(ret);
    }

    void FrameBuffer::setFiltering(D3D11_FILTER filter) {
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
    void FrameBuffer::setFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag, TEXTURE_FILTERING mip) { this->min = min; this->mag = mag; this->mip = mip; setFiltering(getFromFiltering(min, mag, mip)); }
    void FrameBuffer::setFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag) { this->min = min; this->mag = mag; mip = mag; setFiltering(getFromFiltering(min, mag, mip)); }
    void FrameBuffer::setFiltering(TEXTURE_FILTERING filter) { min = mag = mip = filter; setFiltering(getFromFiltering(min, mag, mip)); }

    void FrameBuffer::bindTex() {
        devcon->PSSetShaderResources(0, 1, &resView);
        devcon->PSSetSamplers(0, 1, &sampler);
    }

    void FrameBuffer::drawImmediate(mat3 mat) {
        bindTex();
        util::drawTexture(mat);
    }

    void FrameBuffer::dispose() {
        resView->Release();
        texture->Release();
        sampler->Release();

        depthStencilTex->Release();
        depthStencilView->Release();
        depthStencilState->Release();

        rtv->Release();
    }
}
