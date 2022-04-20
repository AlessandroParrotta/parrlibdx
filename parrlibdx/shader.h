#pragma once

#include <string>
#include <unordered_map>
#include <any>
#include <d3dcompiler.h>

#include <parrlibcore/vector2f.h>
#include <parrlibcore/vector3f.h>
#include <parrlibcore/vector4f.h>
#include <parrlibcore/matrix3f.h>
#include <parrlibcore/matrix4f.h>

#include "common.h"

#include "constantbuffer.h"

namespace prb {

	class Shader {
	private:
		void init(const void* vsByteCode, size_t vsByteSize, const void* psByteCode, size_t psByteSize, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc);
		void init(std::string fileNameVS, std::string filenamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc);
	public:
		ID3D11InputLayout* layout = NULL;
		ID3D11VertexShader* vs = NULL;
		ID3D11PixelShader* ps = NULL;

		cbuf constBuf;
		std::vector<BYTE> uniformData;
		std::unordered_map<std::string, std::pair<size_t, size_t>> uniforms;

		template<typename T> T get(std::string const& name) {
			T res;
			if (uniforms.find(name) == uniforms.end()) return res;

				memcpy(&res, &uniformData[uniforms[name].first], uniforms[name].second);

			return res;
		}

		//std::unordered_map<std::string, std::any> cbufs;
		//template<class T> T get(std::string id) { return std::any_cast<T>(cbufs[id]); }
		//template<class T> T operator[](std::string id) { return get<T>(id); }

		Shader();
		Shader(std::string fileNameVS, std::string fileNamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc);
		Shader(std::string fileNameVS, std::string fileNamePS);
		Shader(std::string fileNameVS, std::string fileNamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> vdesc, bool compileRuntime);

		void setUniform(std::string const& name, const void* val, size_t byteSize);

		void setUniform(std::string const& name, int i);
		void setUniform(std::string const& name, float f);
		void setUniform(std::string const& name, bool b);

		void setUniform(std::string const& name, vec2 const& v);
		void setUniform(std::string const& name, vec3 const& v);
		void setUniform(std::string const& name, vec4 const& v);

		void setUniform(std::string const& name, mat3 const& mat);
		void setUniform(std::string const& name, mat4 const& mat);

		void use() const;
		void release() const;

		void dispose();
	};
}
