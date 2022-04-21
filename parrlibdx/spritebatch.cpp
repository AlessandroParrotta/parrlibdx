#include "SpriteBatch.h"

namespace prb {
	void SpriteBatch::init() {
		//vao = { { { {}, 4 } } }; //this probably leaks memory if you reinitialize the spritebatch
		vao = vbuf(vertBatch{ 0.f }, 4, 0); //this probably leaks memory if you reinitialize the spritebatch
		//sh = { "assets/shaders/spritebatch.vert", "assets/shaders/spritebatch.frag" };
		sh = { "assets/shaders/texturev.cso", "assets/shaders/texturep.cso" };
	}

	SpriteBatch::SpriteBatch() {}
	SpriteBatch::SpriteBatch(Sprite const& atlas) : atlas(atlas) { init(); }

	void SpriteBatch::draw(Sprite const& s, vec2 const& txmin, vec2 const& txmax, mat3 const& transform) {
		if (s.tex.texture != atlas.tex.texture) flushAndClear();

		vec2 v0 = transform * vec2(-1.f, -1.f);
		vec2 v1 = transform * vec2(-1.f, 1.f);
		vec2 v2 = transform * vec2(1.f, 1.f);
		vec2 v3 = transform * vec2(1.f, -1.f);

		if (data.size() < curFloat + 3 * 2 * 4) data.resize(curFloat + 3 * 2 * 4);

		data[curFloat++] = v0.x; data[curFloat++] = v0.y; data[curFloat++] = txmin.x; data[curFloat++] = txmin.y;
		data[curFloat++] = v1.x; data[curFloat++] = v1.y; data[curFloat++] = txmin.x; data[curFloat++] = txmax.y;
		data[curFloat++] = v2.x; data[curFloat++] = v2.y; data[curFloat++] = txmax.x; data[curFloat++] = txmax.y;

		data[curFloat++] = v2.x; data[curFloat++] = v2.y; data[curFloat++] = txmax.x; data[curFloat++] = txmax.y;
		data[curFloat++] = v3.x; data[curFloat++] = v3.y; data[curFloat++] = txmax.x; data[curFloat++] = txmin.y;
		data[curFloat++] = v0.x; data[curFloat++] = v0.y; data[curFloat++] = txmin.x; data[curFloat++] = txmin.y;
	}
	void SpriteBatch::draw(Sprite const& s, Sprite::AnimationPlayer const& sa, mat3 const& transform) { vec2 offset = s.getOffset(sa) / s.tex.getSize(); draw(s, offset, offset + s.size / s.tex.getSize(), transform); }
	void SpriteBatch::draw(Sprite const& s, mat3 const& transform) { draw(s, 0.f, 1.f, transform); }

	void SpriteBatch::flush() {
		vao.setData(data, curFloat);

		sh.use();
		sh.setUniform("transform", transform);
		//sh.setUniform("tex", 0);
		util::bindTexture(atlas.tex);

		vao.draw();
		sh.release();
	}
	void SpriteBatch::clear() { curFloat = 0; }
	void SpriteBatch::flushAndClear() { flush(); clear(); }
}
