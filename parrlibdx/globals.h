#pragma once

#include <string>
#include <unordered_map>

#include "TextRenderer.h"
#include "shader.h"
#include "framebuffer.h"

namespace prb {
	class Sprite;

	namespace globals {
		bool findTxr(std::string const& name);
		void txr(std::string const& name, std::vector<std::string> const& fonts, int const& fontSize);
		void txr(std::string const& name, std::string const& fontName, int const& fontSize);
		TextRenderer& txr(std::string const& name);

		bool findShader(std::string const& name);
		void shader(std::string const& name, Shader const& sh);
		Shader& shader(std::string const& name);

		bool findSprite(std::string const& name);
		void sprite(std::string const& name, Sprite const& s);
		Sprite& sprite(std::string const& name);

		bool findFrameBuffer(std::string const& name);
		void frameBuffer(std::string const& name, FrameBuffer const& fb);
		FrameBuffer& frameBuffer(std::string const& name);

		bool findTexture(std::wstring const& path);
		void setTexture(std::wstring const& path, Texture const& tex);
		void setTexture(std::wstring const& path);
		Texture& getTexture(std::wstring const& path);
	}

#ifndef PARRLIBDX_NAMESPACE_SHORTEN
	namespace glb = globals;
#endif
}
