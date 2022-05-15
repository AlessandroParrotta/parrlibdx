#include "common.h"

#include "stringutils.h"
#include "shader.h"

namespace prb {
    D3D11_FILTER getFromFiltering(TEXTURE_FILTERING min, TEXTURE_FILTERING mag, TEXTURE_FILTERING mip) {
        if (min == LINEAR && mag == LINEAR && mip == LINEAR) return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        else if (min == LINEAR && mag == LINEAR && mip == NEAREST) return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        else if (min == LINEAR && mag == NEAREST && mip == LINEAR) return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        else if (min == LINEAR && mag == NEAREST && mip == NEAREST) return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        else if (min == NEAREST && mag == LINEAR && mip == LINEAR) return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        else if (min == NEAREST && mag == LINEAR && mip == NEAREST) return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        else if (min == NEAREST && mag == NEAREST && mip == LINEAR) return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        else if (min == NEAREST && mag == NEAREST && mip == NEAREST) return D3D11_FILTER_MIN_MAG_MIP_POINT;

        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }

    std::tuple<TEXTURE_FILTERING, TEXTURE_FILTERING, TEXTURE_FILTERING> calcMinMagMip(D3D11_FILTER filter) {
        TEXTURE_FILTERING min, mag, mip;
        if (filter == D3D11_FILTER_MIN_MAG_MIP_LINEAR) { min == LINEAR; mag == LINEAR; mip == LINEAR; }
        else if (filter == D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT) { min == LINEAR; mag == LINEAR; mip == NEAREST; }
        else if (filter == D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR) { min == LINEAR; mag == NEAREST; mip == LINEAR; }
        else if (filter == D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT) { min == LINEAR; mag == NEAREST; mip == NEAREST; }
        else if (filter == D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR) { min == NEAREST; mag == LINEAR; mip == LINEAR; }
        else if (filter == D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT) { min == NEAREST; mag == LINEAR; mip == NEAREST; }
        else if (filter == D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR) { min == NEAREST; mag == NEAREST; mip == LINEAR; }
        else if (filter == D3D11_FILTER_MIN_MAG_MIP_POINT) { min == NEAREST; mag == NEAREST; mip == NEAREST; }
        
        return { min, mag, mip };
    }

    void ThrowIfFailed(HRESULT hr) { if (FAILED(hr)) std::terminate(); }

    // global declarations
    HWND windowHwnd;

    IDXGISwapChain* swapchain = NULL;             // the pointer to the swap chain interface
    ID3D11Device* dev = NULL;                     // the pointer to our Direct3D device interface
    ID3D11DeviceContext* devcon = NULL;           // the pointer to our Direct3D device context
    ID3D11RenderTargetView* backbuffer = NULL;    // the pointer to our back buffer
    //ID3D11InputLayout* pLayout;            // the pointer to the input layout
    //ID3D11VertexShader* pVS;               // the pointer to the vertex shader
    //ID3D11PixelShader* pPS;                // the pointer to the pixel shader
    Shader defTexShader;
    Shader defShader;
    ID3D11Buffer* pVBuffer = NULL;                // the pointer to the vertex buffer
    ID3D11BlendState* g_pBlendStateNoBlend = NULL;

    double deltaTime = 0.;
    double curtime = 0.;

    std::wstring reverse(std::wstring str) {
        std::wstring res;
        for (int i = 0; i < str.length(); i++) res += str[str.length() - 1 - i];
        return res;
    }

    std::wstring getExt(std::wstring str) {
        if (str.find('.') == std::wstring::npos) return str;

        std::wstring rev = reverse(str);
        return reverse(rev.substr(0, rev.find(L'.')));
    }

    std::wstring getFileName(std::wstring str) {
        if (str.find(L'\\') == std::wstring::npos) return str;

        std::wstring rev = reverse(str);
        return reverse(rev.substr(0, rev.find(L'\\')));
    }

    std::string reverse(std::string str) {
        std::string res;
        for (int i = 0; i < str.length(); i++) res += str[str.length() - 1 - i];
        return res;
    }

    std::string getFolder(std::string path) {
        if (path.find('.') == std::string::npos || path.find('\\') == std::string::npos) return path;

        std::string rev = reverse(path);
        return reverse(rev.substr(rev.find('\\'), rev.length()));
    }

    std::string getExeLocation() {
        LPSTR exeloc = new CHAR[1000];
        DWORD nsize = 1000;
        GetModuleFileNameA(NULL, exeloc, nsize);

        std::string res(exeloc);
        delete[] exeloc;

        return res;
    }
    std::string getExeFolder() { return getFolder(getExeLocation()); }

    std::vector<byte> readFile(std::string name) {
        name = strup::fallbackPath(name);

        std::ifstream f(name, std::ios::binary);

        if (!f.is_open() || !f.good()) {
            //throw std::runtime_error("file could not be opened\n"); 
            std::terminate();
        }

        if (f.fail()) std::terminate();

        int length = 0;
        f.seekg(0, f.end);
        length = f.tellg();
        f.seekg(0, f.beg);

        std::vector<byte> bytes;

        char* buf = new char[length];

        f.read(buf, length);
        for (int i = 0; i < length; i++) bytes.push_back((unsigned char)buf[i]);

        delete[] buf;

        if (!f) std::terminate();

        f.close();

        return bytes;
    }

    ID3D11VertexShader* getVertexShaderFromFile(std::string const& path) {
        ID3D11VertexShader* ptr = NULL;
        std::vector<byte> vshader = readFile(path);
        ThrowIfFailed(
            dev->CreateVertexShader(
                &vshader[0],
                vshader.size(),
                nullptr,
                &ptr
            )
        );
        return ptr;
    }
    ID3D11PixelShader* getPixelShaderFromFile(std::string const& path) {
        ID3D11PixelShader* ptr = NULL;
        std::vector<byte> pshader = readFile(path);
        ThrowIfFailed(
            dev->CreatePixelShader(
                &pshader[0],
                pshader.size(),
                nullptr,
                &ptr
            )
        );
        return ptr;
    }
}
