#pragma once

#include <string>
#include <unordered_map>

#include "TextRenderer.h"
#include "shader.h"
#include "framebuffer.h"

namespace prb {
	namespace globals {
		bool findTxr(std::string const& name);
		void setTxr(std::string const& name, std::string const& fontName, int const& fontSize);
		TextRenderer& getTxr(std::string const& name);

		bool findShader(std::string const& name);
		void setShader(std::string const& name, Shader const& sh);
		Shader& getShader(std::string const& name);

		bool findFrameBuffer(std::string const& name);
		void setFrameBuffer(std::string const& name, FrameBuffer const& fb);
		FrameBuffer& getFrameBuffer(std::string const& name);

		bool findTexture(std::wstring const& path);
		void setTexture(std::wstring const& path, Texture const& tex);
		void setTexture(std::wstring const& path);
		Texture& getTexture(std::wstring const& path);
	}

#ifndef PARRLIBDX_NAMESPACE_SHORTEN
	namespace glb = globals;
#endif
}
