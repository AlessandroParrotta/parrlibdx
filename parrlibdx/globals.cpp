#include "globals.h"

#include "sprite.h"

namespace prb {
	namespace globals {
		std::unordered_map<std::string, TextRenderer> txrs;
		bool findTxr(std::string const& name) { return txrs.find(name) != txrs.end(); }
		void txr(std::string const& name, std::vector<std::string> const& fonts, int const& fontSize){ txrs[name] = { fonts, fontSize }; }
		void txr(std::string const& name, std::string const& fontName, int const& fontSize) { txrs[name] = { { fontName }, fontSize }; }
		TextRenderer& txr(std::string const& name) {
			if (txrs.find(name) == txrs.end()) txr(name, "assets/fonts/segoeui.ttf", 24);
			return txrs[name];
		}

		std::unordered_map<std::string, Shader> shaders;
		bool findShader(std::string const& name) { return shaders.find(name) != shaders.end(); }
		void shader(std::string const& name, Shader const& sh) {
			shaders[name] = sh;
		}
		Shader& shader(std::string const& name) {
			if (!findShader(name)) shaders[name] = { (name + "v.hlsl"), (name + "p.hlsl") };
			return shaders[name];
		}

		std::unordered_map<std::string, Sprite> sprites;
		bool findSprite(std::string const& name) { return sprites.find(name) != sprites.end(); }
		void sprite(std::string const& name, Sprite const& s) { sprites[name] = s; }
		Sprite& sprite(std::string const& name) {
			if (sprites.find(name) == sprites.end()) sprites[name] = name.c_str();
			return sprites[name];
		}

		std::unordered_map<std::string, FrameBuffer> fbufs;
		bool findFrameBuffer(std::string const& name) { return shaders.find(name) != shaders.end(); }
		void frameBuffer(std::string const& name, FrameBuffer const& fb) {
			fbufs[name] = fb;
		}
		FrameBuffer& frameBuffer(std::string const& name) {
			if (!findFrameBuffer(name)) fbufs[name] = FrameBuffer({ 1.f,1.f });
			return fbufs[name];
		}

		std::unordered_map<std::wstring, Texture> ftexs;
		bool findTexture(std::wstring const& path) { return ftexs.find(path) != ftexs.end(); }
		void setTexture(std::wstring const& path, Texture const& tex) { ftexs[path] = tex; }
		void setTexture(std::wstring const& path) { ftexs[path] = Texture(path); }
		Texture& getTexture(std::wstring const& path) {
			if (!findTexture(path)) setTexture(path);
			return ftexs[path];
		}
	}

}
