#include "shader.h"

#include <filesystem>

#include <parrlibcore/stringutils.h>
#include <parrlibcore/otherutil.h>

#include "debug.h"

namespace prb {

    void Shader::init(const void* vsByteCode, size_t vsByteSize, const void* psByteCode, size_t psByteSize, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc) {
        ThrowIfFailed(
            dev->CreateVertexShader(
                vsByteCode,
                vsByteSize,
                nullptr,
                &vs
            )
        );

        ThrowIfFailed(
            dev->CreateInputLayout(
                &vdesc[0],
                vdesc.size(),
                vsByteCode,
                vsByteSize,
                &layout
            )
        );

        ThrowIfFailed(
            dev->CreatePixelShader(
                psByteCode,
                psByteSize,
                nullptr,
                &ps
            )
        );
    }

    void Shader::init(std::string fileNameVS, std::string filenamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc) {
        std::vector<byte> vshader = readFile(fileNameVS);
        std::vector<byte> pshader = readFile(filenamePS);
        init((const void*)&vshader[0], vshader.size(), (const void*)&pshader[0], pshader.size(), vdesc);
    }

    Shader::Shader() {}
    Shader::Shader(std::string fileNameVS, std::string fileNamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc) { init(fileNameVS, fileNamePS, vdesc); }
    Shader::Shader(std::string fileNameVS, std::string fileNamePS) {
        std::vector<D3D11_INPUT_ELEMENT_DESC> vdesc =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        init(fileNameVS, fileNamePS, vdesc);
    }

    void manageCompileError(HRESULT hr, ID3DBlob** errorBlob) {
        if (FAILED(hr))
        {
            deb::pr("shader compilation error:\n");
            if (*errorBlob)
            {
                deb::pr(((char*)(*errorBlob)->GetBufferPointer()));
                (*errorBlob)->Release();
            }
            else deb::pr("unknown error (maybe file not found?)");

            //deb::mbe();

            std::terminate();
        }
    }

    Shader::Shader(std::string fileNameVS, std::string fileNamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> vdesc, bool compileRuntime) {
        if (compileRuntime) {
            const D3D_SHADER_MACRO defines[] =
            {
                "EXAMPLE_DEFINE", "1",
                NULL, NULL
            };
            
            //for(int i=0; i<vdesc.size(); i++) deb::debss << vdesc[i].SemanticName << " "; 
            //deb::msbinfo();

            ID3DBlob* vsBlob = 0;
            ID3DBlob* errBlob = 0;
            HRESULT pHResult;
            manageCompileError(D3DX11CompileFromFile(stru::towstr(fileNameVS).c_str(), NULL, NULL, "main", "vs_4_0_level_9_3", 0, 0, NULL, &vsBlob, &errBlob, NULL), &errBlob);

            ID3DBlob* psBlob;
            manageCompileError(D3DX11CompileFromFile(stru::towstr(fileNamePS).c_str(), NULL, NULL, "main", "ps_4_0_level_9_3", 0, 0, NULL, &psBlob, &errBlob, NULL), &errBlob);

            init(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), psBlob->GetBufferPointer(), psBlob->GetBufferSize(), vdesc);
        }
        else init(fileNameVS, fileNamePS, vdesc);
    }

    void Shader::setUniform(std::string const& name, const void* val, size_t byteSize) {
        //for (auto& v : uniforms) {
        //    deb::out("uniform name: ", v.first, ", offset: ", v.second.first, ", size: ", v.second.second, "\n");
        //}

        if (uniforms.find(name) == uniforms.end()) {
            uniforms[name] = { uniformData.size(), byteSize };

            uniformData.reserve(uniformData.size() + byteSize);
            for (int i = 0; i < byteSize; i++) { uniformData.push_back(((BYTE*)val)[i]); }

            //align to 16 bytes
            if (byteSize % 16 != 0) {
                int align = 16 - byteSize % 16; std::vector<BYTE> zeros(align, 0);
                uniformData.insert(uniformData.end(), zeros.begin(), zeros.end());

                //deb::debss << align << " " << uniformData.size() << " " << byteSize << "\n"; deb::outStr();
            }

            //recreate the constant buffer with the updated byte map
            constBuf.dispose();
            constBuf = cbuf(&uniformData[0], uniformData.size());

            //deb::debss << "new uniform set '" << name << "' " << uniforms[name].first << " " << uniforms[name].second;  deb::msbinfo();
        }

        int offset = uniforms[name].first;
        int uniformSize = uniforms[name].second;
        for (int i = 0; i < uniformSize; i++) uniformData[offset + i] = ((BYTE*)val)[i];

        //deb::debss << "BYTEMAP: ";
        //for (int i = 0; i < byteSize; i++) deb::debss << (int)uniformData[offset + i] << " ";
        //deb::debss << " REAL: ";
        //for (int i = 0; i < byteSize; i++) deb::debss << (int)((byte*)val)[i] << " ";
        //deb::debss << "\n"; deb::outStr();

        //constBuf.setData(((byte*)&uniformData[0]) + offset, offset, uniformSize);
        constBuf.setData(&uniformData[0], uniformData.size());

        //deb::out("uniformData size ", uniformData.size(), "\n");

        //deb::debss << offset << " " << uniforms[name].second << "\n"; deb::outStr();
    }

    void Shader::setUniform(std::string const& name, int i) { setUniform(name, (void*)&i, sizeof(int)); }
    void Shader::setUniform(std::string const& name, float f) { setUniform(name, (void*)&f, sizeof(float)); }
    void Shader::setUniform(std::string const& name, bool b) { setUniform(name, (void*)&b, sizeof(bool)); }

    void Shader::setUniform(std::string const& name, vec2 const& v) { setUniform(name, (void*)&v, sizeof(vec2)); }
    void Shader::setUniform(std::string const& name, vec3 const& v) { setUniform(name, (void*)&v, sizeof(vec3)); }
    void Shader::setUniform(std::string const& name, vec4 const& v) { setUniform(name, (void*)&v, sizeof(vec4)); }

    void Shader::setUniform(std::string const& name, mat3 const& mat) { 
        //setUniform(name, (void*)&mat, sizeof(mat3));

        float _0[3]{ mat.m[0][0], mat.m[0][1], mat.m[0][2] }; setUniform("__" + name + "_0_", (void*)&_0, sizeof(_0));
        float _1[3]{ mat.m[1][0], mat.m[1][1], mat.m[1][2] }; setUniform("__" + name + "_1_", (void*)&_1, sizeof(_1));
        float _2[3]{ mat.m[2][0], mat.m[2][1], mat.m[2][2] }; setUniform("__" + name + "_2_", (void*)&_2, sizeof(_2));
    }
    void Shader::setUniform(std::string const& name, mat4 const& mat) { setUniform(name, (void*)&mat, sizeof(mat4)); }

    void Shader::use() const {
        devcon->VSSetShader(vs, 0, 0);
        devcon->PSSetShader(ps, 0, 0);
        devcon->IASetInputLayout(layout);

        if (constBuf.cptr) { constBuf.use(); /*deb::msbinfo("use constbuf");*/ }
    }
    void Shader::release() const {}

    void Shader::dispose() {
        layout->Release();
        vs->Release();
        ps->Release();

        constBuf.dispose();
    }
}
