#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS


#include "debug.h"

#include <locale>
#include <codecvt>
#include <string>

#include <parrlibcore/otherutil.h>

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
		//dss deb;
		//dss rtdeb;
		//std::wstringstream ss;
		//std::wstringstream rtss;
		uss ss;
		std::wstringstream rtss;
		std::vector<std::wstring> strs;

		//void printdeb() {
		//	debs.push_back(deb.str());
		//
		//	deb.clear();
		//	deb.str(L"");
		//}

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
			debtxr = TextRenderer(getExeFolder() + "segoeui.ttf", 26);
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
				for (int i = 0; i < rtdebs.size(); i++) {
					debtxr.drawText(rtdebs[i], { -.99f, .95f - strSpacing * i }, screenSize);
				}
			}

			if (input::getKey(VK_PRIOR)) { //page up
				if (input::getKey(VK_SHIFT))startOffset = (std::min)(-(int)strs.size() + debDrawLimit, 0);
				else if (input::getKey(VK_LCONTROL)) startOffset--;
				else startOffset -= offsetSpeed; offsetSpeed += .005f; rtss << "offset " << startOffset << "/" << strs.size() << "\n";
			}
			else if (input::getKey(VK_NEXT)) { //page down
				if (input::getKey(VK_SHIFT))startOffset = 0;
				else if (input::getKey(VK_LCONTROL)) startOffset++;
				else startOffset += offsetSpeed; offsetSpeed += .005f; rtss << "offset " << startOffset << "/" << strs.size() << "\n";
			}
			else offsetSpeed = 1.f;
			startOffset = outl::clamp(startOffset, (std::min)(-(int)strs.size() + debDrawLimit, 0), debDrawLimit);

			std::vector<std::wstring> debs = getDebs(ss.ss);
			if (debs.size() > 0) strs.insert(strs.end(), debs.begin(), debs.end());
			if (strs.size() > deblimit) strs.erase(strs.begin(), strs.end() - deblimit);
			if (drawStrs) {
				int start = (std::max)((int)strs.size() - debDrawLimit + startOffset, 0);
				int end = (std::min)(start + debDrawLimit, (int)strs.size());
				for (int i = start; i < end; i++) {
					debtxr.drawText(strs[i], { -.99f, strDebStart - strSpacing * (i - start) }, screenSize);
				}
			}
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
