#include "TextRenderer.h"

#include "debug.h"
#include "shader.h"
#include "common.h"

namespace prb {

	void TextRenderer::defInit() {
		vb = vbuf(sizeof(float), sizeof(float) * (2 + 4 + 2), 0);
	}

	TextRenderer::TextRenderer() {}
	TextRenderer::TextRenderer(std::string const& font, int fontSize) {
		this->font = font;
		this->fontSize = fontSize;

		if (FT_Init_FreeType(&library)) {
			deb::msberr("Error: Could not load FreeType library.");
			std::terminate();
		}

		if (FT_New_Face(library, font.c_str(), 0, &face)) {
			deb::debss << "Error: Could not load font '" << font.c_str() << "'."; deb::msberr();
			return;
		}

		FT_Set_Pixel_Sizes(face, 0, fontSize);
		FT_Select_Charmap(face, ft_encoding_unicode);

		g = face->glyph;

		FT_Stroker_New(library, &stroker);
		FT_Outline_New(library, 0xFFFF, 0xFFFF, &outline);

		defInit();
	}

	TextRenderer::Atlas& TextRenderer::getBackAtlas() {
		if (atlases.size() == 0) atlases.push_back(Atlas());
		return atlases.back();
	}

	TextRenderer::glyph TextRenderer::loadGlyph(unsigned int gl) {
		if (glyphs.find(gl) != glyphs.end()) return glyphs[gl];

		FT_UInt idx = FT_Get_Char_Index(face, gl);
		if (idx == 0 && gl != 0) { //could not load glyph, load tofu
			glyph tofu = loadGlyph(0);
			glyphs[gl] = tofu;
		}

		if (FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT)) { deb::msberrw(L"could not load glyph (" + std::to_wstring((wchar_t)gl) + L")"); if (gl == 0) std::terminate(); return loadGlyph(0); }

		FT_Glyph glp;
		if (FT_Get_Glyph(face->glyph, &glp)) return loadGlyph(0);
		if (FT_Glyph_To_Bitmap(&glp, FT_RENDER_MODE_NORMAL, nullptr, true)) return loadGlyph(0);

		FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glp);
		FT_Bitmap ftbitmap = bitmapGlyph->bitmap;

		vec2 glyphSize = vec2(ftbitmap.width, ftbitmap.rows);

		Atlas& at = getBackAtlas();
		vec2 start = getBackAtlas().curMin;

		glyphs[gl].advx = glp->advance.x >> 16;
		glyphs[gl].advy = glp->advance.y >> 16;

		glyphs[gl].bitw = ftbitmap.width;
		glyphs[gl].bitr = ftbitmap.rows;

		glyphs[gl].bitl = bitmapGlyph->left;
		glyphs[gl].bitt = bitmapGlyph->top;

		glyphs[gl].txmin = at.curMin;
		glyphs[gl].txmax = at.curMin + glyphSize;

		if ((wchar_t)gl != L' ') {
			unsigned char* bitmap = ftbitmap.buffer;

			unsigned char* img = new unsigned char[glyphSize.x * glyphSize.y * 4];

			for (int y = 0; y < glyphSize.y; y++) {
				for (int x = 0; x < glyphSize.x; x++) {
					int idx = y * glyphSize.x + x;

					img[idx * 4] = bitmap[idx];
					img[idx * 4 + 1] = bitmap[idx];
					img[idx * 4 + 2] = bitmap[idx];
					img[idx * 4 + 3] = bitmap[idx];
					//deb::outStr(idx * 4, " ", idx * 4 + 3, "\n");
				}
			}

			if (at.tex.getSize().x < at.curMin.x + glyphSize.x ||
				at.tex.getSize().y < at.curMin.y + glyphSize.y) {

				deb::outStr("resize ", at.tex.getSize(), " -> ", (at.curMin + glyphSize + 2.f).maxed(at.tex.getSize()), "\n");

				at.tex.resize((at.curMin + glyphSize + 2.f).maxed(at.tex.getSize()));
			}

			deb::outStr("at.curmin glyphsize ", at.curMin, " ", glyphSize, " [->", at.curMin + glyphSize, "] [", at.tex.getSize(), "]\n");

			at.tex.fillRegion(img, at.curMin, glyphSize);
			at.curMin += vec2{ glyphSize.x, 0.f } + vec2{ 2.f, 0.f };

			//deb::outStr("loaded glyph\n");

			//deb::outStr("img size: ", glyphSize.x * glyphSize.y * 4, "\n");

			delete[] img;

			//deb::outStr("deleted img\n");
		}
	}

	void TextRenderer::preloadGlyphs(std::wstring const& str) {
		Atlas& at = getBackAtlas();
		for (int i = 0; i < str.length(); i++) {
			//deb::outStr(str[i], "\n");
			if (glyphs.find(str[i]) == glyphs.end()) { 
				glyph gp;
				//deb::outStr("points size: ", gp.points.size(), "\n");
				gp = loadGlyph(str[i]);
			}
			//deb::outStr("ldoeaddo\n");
		}
	}

	aabb2 TextRenderer::getTextBoundingBox(vec2 const& pos, vec2 screenSize) {
		std::wstring str = tss.str();
		tss.clear();
		tss.str(L"");

		preloadGlyphs(str);

		vec2 cur = (pos + 1.f) / 2.f * screenSize;
		aabb2 bb = pos;

		int curData = 0;

		for (int i = 0; i < str.length(); i++) {
			glyph g = glyphs[str[i]];

			vec2 pos = -1.f, txmin;
			vec2 size = 2.f, txmax;

			pos = cur + vec2(g.bitl, -(g.bitr - g.bitt)) - txBias;
			size = vec2(g.bitw, g.bitr) + txBias * 2.f;

			txmin = g.txmin - txBias;
			txmax = g.txmax + txBias;

			cur += vec2(g.advx, g.advy);

			//pixel to perc conversions
			pos = vec2{ pos.x / screenSize.x, pos.y / screenSize.y }*2.f - 1.f;
			size = vec2{ size.x / screenSize.x, size.y / screenSize.y }*2.f;
			//pos /= screenSize.aspectmaxv();
			//size /= screenSize.aspectmaxv();

			bb = bb.rescaled(pos);
			bb = bb.rescaled(pos + size);
		}

		return bb;
	}

	void TextRenderer::batchText(TextRenderer::batchedText& res, vec2 const& pos, vec2 screenSize) {
		std::wstring str = tss.str();
		tss.clear();
		tss.str(L"");

		preloadGlyphs(str);

		vec2 cur = (pos + 1.f) / 2.f * screenSize;
		res.bb = pos;

		res.vdata.resize(str.length() * 2 * 3 * (2 * 4 * 2));
		int curData = 0;

		for (int i = 0; i < str.length(); i++) {
			glyph g = glyphs[str[i]];

			vec2 pos = -1.f, txmin;
			vec2 size = 2.f, txmax;

			pos = cur + vec2(g.bitl, -(g.bitr - g.bitt)) - txBias;
			size = vec2(g.bitw, g.bitr) + txBias * 2.f;

			txmin = g.txmin - txBias;
			txmax = g.txmax + txBias;

			cur += vec2(g.advx, g.advy);

			//pixel to perc conversions
			pos = vec2{ pos.x / screenSize.x, pos.y / screenSize.y }*2.f - 1.f;
			size = vec2{ size.x / screenSize.x, size.y / screenSize.y }*2.f;
			//pos /= screenSize.aspectmaxv();
			//size /= screenSize.aspectmaxv();

			res.bb = res.bb.rescaled(pos);
			res.bb = res.bb.rescaled(pos + size);

			txmin /= getBackAtlas().tex.getSize();
			txmax /= getBackAtlas().tex.getSize();

			vec2 v0 = pos;
			vec2 v1 = pos + size.oy();
			vec2 v2 = pos + size;
			vec2 v3 = pos + size.xo();

			//debss << v0 << " " << v1 << " " << v2 << " " << v3; msbinfo();

			vec2 tx0 = { txmin.x, txmax.y };
			vec2 tx1 = txmin;
			vec2 tx2 = { txmax.x, txmin.y };
			vec2 tx3 = txmax;

			//debss << tx0 << " " << tx1 << " " << tx2 << " " << tx3; msbinfo();

			res.vdata[curData++] = v0.x; res.vdata[curData++] = v0.y;		res.vdata[curData++] = color.x; res.vdata[curData++] = color.y; res.vdata[curData++] = color.z; res.vdata[curData++] = color.w;			res.vdata[curData++] = tx0.x; res.vdata[curData++] = tx0.y;
			res.vdata[curData++] = v1.x; res.vdata[curData++] = v1.y;		res.vdata[curData++] = color.x; res.vdata[curData++] = color.y; res.vdata[curData++] = color.z; res.vdata[curData++] = color.w;			res.vdata[curData++] = tx1.x; res.vdata[curData++] = tx1.y;
			res.vdata[curData++] = v2.x; res.vdata[curData++] = v2.y;		res.vdata[curData++] = color.x; res.vdata[curData++] = color.y; res.vdata[curData++] = color.z; res.vdata[curData++] = color.w;			res.vdata[curData++] = tx2.x; res.vdata[curData++] = tx2.y;

			res.vdata[curData++] = v2.x; res.vdata[curData++] = v2.y;		res.vdata[curData++] = color.x; res.vdata[curData++] = color.y; res.vdata[curData++] = color.z; res.vdata[curData++] = color.w;			res.vdata[curData++] = tx2.x; res.vdata[curData++] = tx2.y;
			res.vdata[curData++] = v3.x; res.vdata[curData++] = v3.y;		res.vdata[curData++] = color.x; res.vdata[curData++] = color.y; res.vdata[curData++] = color.z; res.vdata[curData++] = color.w;			res.vdata[curData++] = tx3.x; res.vdata[curData++] = tx3.y;
			res.vdata[curData++] = v0.x; res.vdata[curData++] = v0.y;		res.vdata[curData++] = color.x; res.vdata[curData++] = color.y; res.vdata[curData++] = color.z; res.vdata[curData++] = color.w;			res.vdata[curData++] = tx0.x; res.vdata[curData++] = tx0.y;

			//debssw << "wchar: " << str[i] << " (" << i << "/" << str.length() << ") done."; msbinfow();
		}

		if (res.vdata.size() == 0) return;
	}
	TextRenderer::batchedText TextRenderer::batchText(vec2 const& pos, vec2 screenSize) { batchedText res; batchText(res, pos, screenSize); return res; }

	void TextRenderer::drawText(vec2 const& pos, vec2 screenSize) {
		batchedText btc; batchText(btc, pos, screenSize);
		if (btc.vdata.size() == 0) return;

		//vbuf vb(btc.vdata, sizeof(float)*(2+4+2), 0);
		vb.setData(btc.vdata);
		getBackAtlas().tex.bind();
		defTexShader.use();
		vb.draw();
		//vb.dispose();
	}

	void TextRenderer::drawText(std::wstring const& str, vec2 const& pos, vec2 screenSize) {
		tss << str;
		drawText(pos, screenSize);
	}

	void TextRenderer::dispose() {
		vb.dispose();
	}
}
