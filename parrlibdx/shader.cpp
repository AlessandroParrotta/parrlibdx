#include "shader.h"

namespace prb {
    void Shader::init(std::string fileNameVS, std::string filenamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc) {
        std::vector<byte> vshader = readFile(fileNameVS);
        ThrowIfFailed(
            dev->CreateVertexShader(
                &vshader[0],
                vshader.size(),
                nullptr,
                &vs
            )
        );

        ThrowIfFailed(
            dev->CreateInputLayout(
                &vdesc[0],
                vdesc.size(),
                &vshader[0],
                vshader.size(),
                &layout
            )
        );

        std::vector<byte> pshader = readFile(filenamePS);
        ThrowIfFailed(
            dev->CreatePixelShader(
                &pshader[0],
                pshader.size(),
                nullptr,
                &ps
            )
        );
    }

    Shader::Shader() {}
    Shader::Shader(std::string fileNameVS, std::string filenamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc) { init(fileNameVS, filenamePS, vdesc); }
    Shader::Shader(std::string fileNameVS, std::string filenamePS) {
        std::vector<D3D11_INPUT_ELEMENT_DESC> vdesc =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        init(fileNameVS, filenamePS, vdesc);
    }
    Shader::Shader(std::string fileNameVS, std::string filenamePS, bool compileRuntime) {
        //TODO
    }

    void Shader::use() {
        devcon->VSSetShader(vs, 0, 0);
        devcon->PSSetShader(ps, 0, 0);
        devcon->IASetInputLayout(layout);
    }
    void Shader::release() {}

    void Shader::dispose() {
        layout->Release();
        vs->Release();
        ps->Release();
    }
}
