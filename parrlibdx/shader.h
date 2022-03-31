#pragma once

#include <string>
#include <unordered_map>
#include <any>

#include "common.h"

namespace prb {
	class Shader {
	private:
		void init(std::string fileNameVS, std::string filenamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc);

	public:
		ID3D11InputLayout* layout = NULL;
		ID3D11VertexShader* vs = NULL;
		ID3D11PixelShader* ps = NULL;

		std::unordered_map<std::string, std::any> cbufs;
		template<class T> T get(std::string id) { return std::any_cast<T>(cbufs[id]); }
		template<class T> T operator[](std::string id) { return get<T>(id); }

		Shader();
		Shader(std::string fileNameVS, std::string filenamePS, std::vector<D3D11_INPUT_ELEMENT_DESC> const& vdesc);
		Shader(std::string fileNameVS, std::string filenamePS);
		Shader(std::string fileNameVS, std::string filenamePS, bool compileRuntime);


		void use();
		void release();

		void dispose();
	};
}
