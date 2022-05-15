#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS


#include "debug.h"

#include <locale>
#include <codecvt>
#include <string>

#include <parrlibcore/otherutil.h>
#include <parrlibcore/tick.h>

#include "Input.h"

#include "common.h"

namespace prb {
	namespace debug {

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::vector<std::wstring> debug;
		std::stringstream debss;
		std::wstringstream debssw;

		std::wstringstream tss;

		bool drawStrs = true;
		int deblimit = 5000;


		TextRenderer debtxr;
		std::mutex dmtx;
		uss ss;
		std::wstringstream rtss;
		std::vector<std::wstring> strs;

		bool windowEnabled = true;

		outl::AsyncProgram prog;
		HWND debHwnd = NULL;
		LRESULT sendMessage(HWND hWnd, std::wstring str) {
			LPCWSTR sstr = str.c_str();
			COPYDATASTRUCT cds;
			cds.dwData = 8952;
			cds.cbData = sizeof(WCHAR) * (str.length() + 1);
			cds.lpData = (PVOID)sstr;
			//deb::ss.ss << "sending  '" << str << "' '" << sstr << "'\n";
			return SendMessage(hWnd, WM_COPYDATA, (WPARAM)(HWND)hWnd, (LPARAM)(LPVOID)&cds);
		}
		LRESULT sendMessage(std::wstring str) { return sendMessage(debHwnd, str); }

		void createWindow() {
			if (windowEnabled) {
				std::wstring path = L"F:\\Programming\\VisualStudio\\DX11\\DebugWindow\\x64\\Release\\DebugWindow.exe";
				prog = outl::openProgramAsync(path, L"");
				//deb::ss.ss << prog.dwProcessID << " " << prog.dwThreadID << "\n";

				for (int i = 0; i < 10; i++) {
					std::this_thread::sleep_for(std::chrono::milliseconds(2));

					std::wstringstream ss;
					std::wstring _class; ss << prog.dwProcessID << "_Class"; _class = ss.str(); ss.str(L""); ss.clear();
					std::wstring name; ss << prog.dwProcessID << "_DebugHidden"; name = ss.str(); ss.str(L""); ss.clear();

					//deb::ss.ss << "looking for " << _class << " " << name << "\n";

					//debHwnd = otherutil::findWindowByClassName(L"WindowClass", L"Debug Window");
					debHwnd = otherutil::findWindowByClassName(_class, name);
					if (debHwnd != (HWND)0) break;
					//deb::ss.ss << "tried getting window " << i << "\n";
				}
			}
		}

		void debInit() {
			//deb::out("initializing deb textrenderer\n");
			//debtxr = TextRenderer({ getExeFolder() + "segoeui.ttf" }, 26);
			debtxr = { { ("assets/fonts/segoeui.ttf") }, 18 }; //(int)(prc::res().y * 0.02f) };
			debtxr.color(vc4::white);
			debtxr.setOutline(2);
			debtxr.setFiltering(NEAREST, NEAREST);
			//deb::out("initialized deb textrenderer\n");
			//deb::out("debtxr tex size ", debtxr.getBackAtlas().tex.getSize(), "\n");
		}

		void dispose() {
			if (windowEnabled) {
				outl::closeProgramAsync(prog);
			}
		}

		std::wstring replaceCharWithString(std::wstring str, wchar_t c, std::wstring rstr) {
			std::wstring res;

			for (int i = 0; i < str.length(); i++) {
				if (str[i] == c) {
					res += rstr;
				}
				else res += str[i];
			}

			return res;
		}

		std::vector<std::wstring> getDebs(std::wstringstream& wss) {

			std::wstring bigstr = wss.str();
			bigstr = replaceCharWithString(bigstr, L'\t', L"    ");

			//std::vector<std::wstring> strs = stru::toLines(wss.str());
			
			//wss.clear();
			//wss.str(L"");

			std::vector<std::wstring> strs;

			bool foundNewLine = false;
			//int len = bigstr.length();
			for (int i = 0; i < bigstr.length(); i++) {
				if (bigstr[i] == L'\n') {
					if (i == 0) { strs.push_back(L""); bigstr.erase(bigstr.begin(), bigstr.begin() + 1); i = 0; }
					else {
						strs.push_back(bigstr.substr(0, i));
						bigstr.erase(bigstr.begin(), bigstr.begin() + i + 1);
						i = 0;
						//debssw << strs.back() << " " << bigstr << " " << bigstr.length(); msbinfow();
					}
					foundNewLine = true;
				}
			}

			if (foundNewLine) {
				wss.clear();
				wss.str(L"");
			}

			return strs;
		}

		float strSpacing = .04f;
		float strDebStart = .575f;
		int startOffset = 0;
		float offsetSpeed = 1.f;
		int debDrawLimit = 40;
		void drawDebStrs(vec2 screenSize) {
			//debssw << deb.str() << "\n" << rtdeb.str(); msbinfow();

			std::lock_guard<std::mutex> lck(dmtx);

			std::vector<std::wstring> rtdebs = getDebs(rtss);
			if (drawStrs) {
				//for (int i = 0; i < rtdebs.size(); i++) {
				//	debtxr.drawText(rtdebs[i], { -.99f, .95f - strSpacing * i }, screenSize);
				//}

				float aspectCorrect = cst::resaspect();
				float scaleFactor = 1.f;
				float cheight = (debtxr.getFontSize() + debtxr.getOutline()) / cst::res().y * (cst::res().aspectmaxv().y * 2.f);

				//deb::out(aspectCorrect, " ", cheight, "\n");

				for (int i = 0; i < rtdebs.size(); i++) {
					//float y = 1.0f - cheight * i;
					//vec2 screen = cst::res().aspectmaxv();
					//vec2 pos = screen.nx() - vec2y(cheight * i);
					vec2 pos = vec2(util::getScreenLeft(), .99f) - vec2y(cheight * i);

					//deb::out(screen, " ", pos, "\n");

					//if (pos.y >= -screen.y && pos.y <= screen.y) {
					if (pos.y >= -1.f && pos.y <= 1.f) {
						//txr.drawWStringpx(rtStrs[i], vec2(1.f), pmat3::translate(vec2(-prc::wres().aspectmax(), y)));
						//txr.drawWStringpx(rtStrs[i], vec2(1.f).ny(), pmat3::translate(vec2(-std::fmax(prc::wres().aspectx(),1.f),y)));
						debtxr.drawWStringpx(rtdebs[i], vec2(1.f).ny(), pmat3::translate(pos));
						//std::cout << "drawStrRT\n";
					}

					//rtdebs[i] = L"";
				}
			}

			if (input::getKey(VK_PRIOR)) { //page up
				if (input::getKey(VK_SHIFT))startOffset = (std::min)(-(int)strs.size() + debDrawLimit, 0);
				else if (input::getKey(VK_LCONTROL)) startOffset--;
				else startOffset += offsetSpeed; offsetSpeed += .005f; rtss << "offset " << startOffset << "/" << strs.size() << "\n";
			}
			else if (input::getKey(VK_NEXT)) { //page down
				if (input::getKey(VK_SHIFT))startOffset = 0;
				else if (input::getKey(VK_LCONTROL)) startOffset++;
				else startOffset -= offsetSpeed; offsetSpeed += .005f; rtss << "offset " << startOffset << "/" << strs.size() << "\n";
			}
			else offsetSpeed = 1.f;
			//startOffset = outl::clamp(startOffset, (std::min)(-(int)strs.size() + debDrawLimit, 0), debDrawLimit);
			startOffset = outl::clamp(startOffset, 0, strs.size());

			std::vector<std::wstring> debs = getDebs(ss.ss);
			if (debs.size() > 0) strs.insert(strs.end(), debs.begin(), debs.end());
			if (strs.size() > deblimit) strs.erase(strs.begin(), strs.end() - deblimit);
			if (drawStrs) {
				//int start = (std::max)((int)strs.size() - debDrawLimit + startOffset, 0);
				//int end = (std::min)(start + debDrawLimit, (int)strs.size());
				//for (int i = start; i < end; i++) {
				//	//deb::out(strDebStart, " ", strSpacing, " ", strDebStart - strSpacing * (i - start), "\n");
				//	debtxr.drawText(strs[i], { -.99f, strDebStart - strSpacing * (i - start) });
				//}

				float aspectCorrect = cst::resaspect();
				float scaleFactor = 1.f;
				float cheight = (debtxr.getFontSize() + debtxr.getOutline()) / cst::res().y * (cst::res().aspectmaxv().y * 2.f);
				
				//float sliderVal = startOffset / 2.0f;
				float sliderVal = 0.f;

				if (cheight * strs.size() < 1.0f) {
					sliderVal = 0.0f;
				}

				startOffset = outl::imax(startOffset, 0);

				for (int i = strs.size() - 1 - startOffset; i >= 0; i--) {
					float ypos = -0.99f + cheight * (strs.size() - 1 - i - startOffset) - sliderVal;

					if (ypos < -1.0f - cheight) continue;
					if (ypos >= 0.0f) break;


					//debtxr.drawWStringpx(strs[i], vec2(1.f), pmat3::translate(vec2(-1.f, ypos)));
					debtxr.drawWStringpx(strs[i], vec2(1.f), pmat3::translate(vec2(util::getScreenLeft(), ypos)));
					//std::cout << "drawStr\n";
				}
			}

			//for (int i = 0; i < 10; i++)
			//	for (int j = 0; j < 10; j++) debtxr.drawText(vec2{ -.99f, strDebStart - strSpacing * (i + j - 0) }, "Hello ", i, " Hi ", j, " tryna get a longer string to see bugs");
		}

		std::wstring tos(int i) { return std::to_wstring(i); }
		std::wstring tos(unsigned int i) { return std::to_wstring(i); }
		std::wstring tos(float i) { return std::to_wstring(i); }
		std::wstring tos(double i) { return std::to_wstring(i); }
		std::wstring tos(const wchar_t* c) { return std::wstring(c); }
		std::wstring tos(const char* c) { return converter.from_bytes(c); }
		std::wstring tos(std::string str) { std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter; return converter.from_bytes(str); }

		std::u32string to_utf32(const std::string& s)
		{
			std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
			return conv.from_bytes(s);
		}

		std::string toutf8(std::wstring wstr) { return converter.to_bytes(wstr); }

		void pdebug(std::wstring str) {
			if (debug.size() > 10) debug.erase(debug.begin());
			debug.push_back(str);
		}

		void msberr(std::string errstr) { MessageBox(0, tos(errstr).c_str(), L"Error", MB_ICONERROR); }
		void msberr() { msberr(debss.str()); debss.clear(); debss.str(""); }

		void msbinfo(std::string infostr) { MessageBox(0, tos(infostr).c_str(), L"Info", MB_ICONINFORMATION); }
		void msbinfo() { msbinfo(debss.str()); debss.clear(); debss.str(""); }


		void msberrw(std::wstring errstr) { MessageBox(0, errstr.c_str(), L"Error", MB_ICONERROR); }
		void msberrw() { msberrw(debssw.str()); debssw.clear(); debssw.str(L""); }

		void msbinfow(std::wstring infostr) { MessageBox(0, infostr.c_str(), L"Info", MB_ICONINFORMATION); }
		void msbinfow() { msbinfow(debssw.str()); debssw.clear(); debssw.str(L""); }

		std::wstring tows(const wchar_t* c) { return std::wstring(c); }
		std::wstring tows(const char* c) { return converter.from_bytes(c); }
		std::wstring tows(std::string const& str) { /*std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;*/ return converter.from_bytes(str); }

		std::u32string toutf32(const std::string& s)
		{
			std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
			return conv.from_bytes(s);
		}

		std::string tos(std::wstring wstr) { return converter.to_bytes(wstr); }

		void mbe(std::wstring const& errstr) { MessageBox(0, errstr.c_str(), L"Error", MB_ICONERROR); }
		void mbe(std::string const& errstr) { mbe(tows(errstr)); }
		void mbe() { mbe(tss.str()); tss.clear(); tss.str(L""); }

		void mbi(std::wstring const& infostr) { MessageBox(0, infostr.c_str(), L"Info", MB_ICONINFORMATION); }
		void mbi(std::string const& infostr) { mbi(tows(infostr)); }
		void mbi() { mbi(tss.str()); tss.clear(); tss.str(L""); }


		void out(std::wstring const& str) { std::wcout << str; OutputDebugString(str.c_str()); }
		void out() { std::wstring str = tss.str(); out(str); tss.clear(); tss.str(L""); }
	}
}
