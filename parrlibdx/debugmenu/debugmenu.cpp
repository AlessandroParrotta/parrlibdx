#include "debugmenu.h"

#include "../util.h"
#include "../context.h"
#include "../debug.h"

namespace prb {
	namespace debugmenu {
		bool enabled = false;
		int page = 4;
		float centerSpacing = .03f;
		float headerHeight = .1f;
		float headerAreaSpacing = .03f;

		debug::console cons;
		void addConsoleCommand(std::wstring name, std::wstring desc, std::vector<debug::fParam> params, std::function<std::wstring(std::vector<std::wstring>)> func) { cons.addCommand(name, desc, params, func); }
		void addConsoleCommand(std::wstring pattern, std::wstring desc, std::function<std::wstring(std::vector<std::wstring>)> func) { cons.addCommand(pattern, desc, func); }


		//---------------------------PAGES---------------------
		int pagenum = 5;

		std::vector<std::wstring> vopts; //video options
		std::vector<vec2> voptssizes;	//actual video resolutions

		std::vector<std::wstring> fopts = { L"Linear", L"Nearest" }; //filtering 
		std::vector<std::wstring> sopts = { L"One to one", L"Integer ratio", L"Floating ratio", L"Fill" }; //scaling 
		void video() {
			imui::setLayout(imui::VERTICAL);
			imui::setSize(vec2(util::getResbbNDCAspect().size().x, .13f));
			imui::setOffset(imui::state.size.ny());

			float starty = util::getResbbNDCAspect()[1].y - headerHeight - imui::state.size.y / 2.f - headerAreaSpacing;

			imui::setPos(vec2(
				-centerSpacing - imui::state.size.x / 2.f,
				starty
			));

			imui::labelBound(L"Resolution:", vec2x(-1.f));
			imui::labelBound(L"Resolution X:", vec2x(-1.f));
			imui::labelBound(L"Resolution Y:", vec2x(-1.f));
			imui::labelBound(L"vSync:", vec2x(-1.f));
			imui::labelBound(L"Framerate Cap:", vec2x(-1.f));
			imui::labelBound(L"Filtering:", vec2x(-1.f));
			imui::labelBound(L"Scaling:", vec2x(-1.f));


			imui::setSize(vec2(.6f, .12f));
			//imui::setOffset(imui::state.size.ny());

			imui::setPos(vec2(centerSpacing + imui::state.size.x / 2.f, starty));

			int res = imui::comboBox("__pardebugres", L"Resolution", vopts);
			if (res > -1) {
				prc::resize(voptssizes[res]);
			}

			int resx = prc::res().x; if (imui::textFieldi("__parresx", &resx) == 3) {
				prc::resize(vec2(resx, prc::res().y));
			}

			int resy = prc::res().y; if (imui::textFieldi("__parresy", &resy) == 3) {
				prc::resize(vec2(prc::res().x, resy));
			}

			vec2 siz = imui::state.size; imui::setSize(siz.y);
			bool vsync = prc::getVSync(); if (imui::checkBox("__parrvsync", &vsync)) {
				prc::setVSync(vsync);
			}
			imui::setSize(siz);

			int fpscap = prc::getFramerateCap(); if (imui::textFieldi("__parrfpscap", &fpscap) == 3) {
				prc::setFramerateCap(fpscap);
			}

			int rfilt = imui::comboBox("__parrfilt", L"Filtering", fopts, 1);
			if (rfilt > -1) {
				prc::setFiltering(rfilt == 0 ? LINEAR : NEAREST);
			}

			int rscal = imui::comboBox("__parrscal", L"Scaling", sopts, prc::scalingMode());
			if (rscal > -1) {
				prc::scalingMode(rscal);
			}
		}

		void input() {

		}

		void debug() {
			imui::setLayout(imui::VERTICAL);
			imui::setSize(vec2(util::getResbbNDCAspect().size().x, .13f));
			imui::setOffset(imui::state.size.ny());

			float starty = util::getResbbNDCAspect()[1].y - headerHeight - imui::state.size.y / 2.f - headerAreaSpacing;

			imui::setPos(vec2(
				-centerSpacing - imui::state.size.x / 2.f,
				starty
			));

			imui::labelBound(L"Enable On-Screen Messages:", vec2x(-1.f));
			imui::labelBound(L"Enable Real-Time Messages:", vec2x(-1.f));
			imui::labelBound(L"Enable Log Messages:", vec2x(-1.f));
			imui::labelBound(L"Show FPS:", vec2x(-1.f));

			imui::setSize(vec2(.6f, .12f));
			//imui::setOffset(imui::state.size.ny());

			imui::setPos(vec2(centerSpacing + imui::state.size.x / 2.f, starty));

			vec2 siz = imui::state.size; imui::setSize(siz.y);
			bool drawStrs = deb::drawStrs; if (imui::checkBox("__parrdrawStrs", &drawStrs)) {
				deb::drawStrs = drawStrs;
			}

			bool RTmsgs = deb::drawRT; if (imui::checkBox("__parrRTMsgs", &RTmsgs)) {
				deb::drawRT = RTmsgs;
			}

			siz = imui::state.size; imui::setSize(siz.y);
			bool msgs = deb::drawMsgs; if (imui::checkBox("__parrMsgs", &msgs)) {
				deb::drawMsgs = msgs;
			}

			bool showFPS = deb::showFPS; if (imui::checkBox("__parrFPS", &showFPS)) {
				deb::showFPS = showFPS;
			}
			imui::setSize(siz);
		}

		void audio() {
			imui::setLayout(imui::VERTICAL);
			imui::setSize(vec2(util::getResbbNDCAspect().size().x, .13f));
			imui::setOffset(imui::state.size.ny());

			float starty = util::getResbbNDCAspect()[1].y - headerHeight - imui::state.size.y / 2.f - headerAreaSpacing;

			imui::setPos(vec2(
				-centerSpacing - imui::state.size.x / 2.f,
				starty
			));

			imui::labelBound(L"Volume:", vec2x(-1.f));


			imui::setSize(vec2(.6f, .12f));
			//imui::setOffset(imui::state.size.ny());

			imui::setPos(vec2(centerSpacing + imui::state.size.x / 2.f, starty));


			float vol = 0.f;
			imui::sliderf("__parrvolume", &vol, 0.f, 1.f, false);
		}

		void vconsole() {
			cons.update();

			cons.draw();
		}

		//-----------------------------

		void init() {
			for (int i = 0; i < prc::vidmodes.size(); i++) {
				if (voptssizes.size() > 0 &&
					(int)voptssizes.back().x == prc::vidmodes[i].res.x &&
					(int)voptssizes.back().y == prc::vidmodes[i].res.y) continue;

				vopts.push_back(stru::fmtw(prc::vidmodes[i].res.x, "x", prc::vidmodes[i].res.y));
				voptssizes.push_back(vec2(prc::vidmodes[i].res.x, prc::vidmodes[i].res.y));
			}

			cons.init(aabb2s(2.f));
		}

		void update() {
			imui::beginContainer("__parrimuidebugmenu", { -100000.f, 100000.f }); //TODO: cheesy hack for containers
			util::setColor(vc4::gray.colMul(.5f).a(.5f));
			util::drawQuad(util::getResbbNDCAspect());

			vec2 hsize = { util::getResbbNDCAspect().size().x / (float)pagenum, headerHeight };
			imui::setLayout(imui::HORIZONTAL);
			imui::setSize(hsize);
			imui::setPos(util::getResbbNDCAspect()[1] + imui::state.size.ny() / 2.f);
			imui::state.offset = imui::state.size;

			util::setColor(vc4::white);
			if (imui::button(L"Video"))		page = 0;
			if (imui::button(L"Input"))		page = 1;
			if (imui::button(L"Debug"))		page = 2;
			if (imui::button(L"Audio"))		page = 3;
			if (imui::button(L"Console"))	page = 4;

			imui::setLayout(imui::VERTICAL);

			switch (page) {
			case 0: video(); break;
			case 1: input(); break;
			case 2: debug(); break;
			case 3: audio(); break;
			case 4: vconsole(); break;
			}

			imui::endContainer();
		}

		void dispose() {

		}
	}
}
