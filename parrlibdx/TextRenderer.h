#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <functional>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

#include <parrlibcore/vector2f.h>
#include <parrlibcore/vector4f.h>
#include <parrlibcore/utils2d/axisalignedboundingbox2d.h>
#include <parrlibcore/stringutils.h>

#include "FlowTexture.h"
#include "vertexbuffer.h"

namespace prb {

class TextRenderer {
public:
	struct glyph {
		float advx; // advance.x
		float advy; // advance.y

		float bitw = 5.f; // bitmap.width;
		float bitr = 5.f; // bitmap.rows;

		float bitl; // bitmap_left;
		float bitt; // bitmap_top;

		vec2 txmin, txmax;		// offset of glyph in texture coordinates
		int atlas = 0;			// which atlas this glyph is on
		bool loaded = true;

		//std::vector<vec2> points; //all points that define the outline of this glyph
	};

	struct Atlas {
		vec2 curMin = 2.f, curMax = 2.f;
		FlowTexture tex;
		Atlas() { tex = FlowTexture(vec2{ 1.f,1.f }); }
	};
	std::vector<Atlas> atlases;

	std::unordered_map<unsigned int, glyph> glyphs; //glyph information

	FT_Library library;
	int fontSize = 16;
	vec4 color = 1.f;

	//font info
	std::string font;
	FT_Face face;
	FT_GlyphSlot g;
	FT_Stroker stroker;
	FT_Outline outline;
	
	std::wstringstream tss; //text string stream, to accumulate text to then flush

	vec2 txBias = 0.5f;
	vbuf vb;

	void defInit();

	TextRenderer();
	TextRenderer(std::string const& font, int fontSize);

	Atlas& getBackAtlas();
	glyph loadGlyph(unsigned int glyph);
	void preloadGlyphs(std::wstring const& str);
	
	aabb2 getTextBoundingBox(vec2 const& pos, vec2 screenSize);

	struct batchedText {
		std::vector<float> vdata;
		aabb2 bb;
	};
	void batchText(batchedText& res, vec2 const& pos, vec2 screenSize); //use tss to fill the text string then use this function to show it on screen
	batchedText batchText(vec2 const& pos, vec2 screenSize); //use tss to fill the text string then use this function to show it on screen
	void drawText(vec2 const& pos, vec2 screenSize); //use tss to fill the text string then use this function to show it on screen
	void drawText(std::wstring const& str, vec2 const& pos, vec2 screenSize); //use tss to fill the text string then use this function to show it on screen

	template<typename... Args> batchedText batchText(vec2 const& pos, Args... args) { tss << stru::fmt(args...); return batchText(pos, cst::res()); }
	template<typename... Args> void drawText(vec2 const& pos, Args... args) { drawText(stru::fmt(args...), pos, cst::res()); }

	template<typename... Args> batchedText batchText(std::function<vec2(aabb2 const&)> afterPos, Args... args) { std::wstring str = stru::fmt(args...); tss << str; aabb2 bb = getTextBoundingBox(0.f, cst::res()); vec2 newPos = afterPos(bb); tss << str; return batchText(newPos, cst::res()); }
	template<typename... Args> void drawText(std::function<vec2(aabb2 const&)> afterPos, Args... args) { std::wstring str = stru::fmt(args...); tss << str; aabb2 bb = getTextBoundingBox(0.f, cst::res()); vec2 newPos = afterPos(bb); tss << str; drawText(newPos, cst::res()); }

	void dispose();
};
}
