#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <sstream>
#include <mutex>

#include <parrlib/math/stringutils.h>

#include "TextRenderer.h"

namespace prb {
	namespace debug {

		extern std::vector<std::wstring> debug;
		extern std::stringstream debss;
		extern std::wstringstream debssw;

		extern std::wstringstream tss;

		extern TextRenderer debtxr;
		extern bool drawStrs;
		extern int deblimit;


		extern bool windowEnabled;

		LRESULT sendMessage(std::wstring str);

		void createWindow();

		void debInit();
		void dispose();

		extern std::mutex dmtx;
		//class dss : public std::wstringstream {
		//public:
		//	//template<class T>
		//	//friend dss& operator<< (std::ostream& ot, T t);
		//
		//	dss& operator<< (const wchar_t* t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (const char* t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (int t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (double t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (float t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (unsigned int t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (short t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (unsigned short t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//	dss& operator<< (size_t t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }
		//};
		//template<class T>
		//dss& operator<< (std::ostream& ot, T t) { std::unique_lock<std::mutex> lck(dmtx); std::wstringstream::operator<< (t); return *this; }

		class uss {
		public:
			std::wstringstream ss;
			std::wstringstream tss;
			template<class T> uss& operator<< (T val) {
				if (windowEnabled) {
					tss << val;
					if (tss.str().find('\n' != std::string::npos)) {
						sendMessage(tss.str()); tss.str(L""); tss.clear();
					}
				}
				else ss << val;

				return *this;
			}
		};

		extern uss ss;
		extern std::wstringstream rtss;
		//void printdeb();
		void drawDebStrs(vec2 screenSize);

		std::wstring tos(int i);
		std::wstring tos(unsigned int i);
		std::wstring tos(float i);
		std::wstring tos(double i);
		std::wstring tos(const wchar_t* c);
		std::wstring tos(const char* c);
		std::wstring tos(std::string str);

		std::u32string to_utf32(const std::string& s);

		std::string toutf8(std::wstring wstr);

		void pdebug(std::wstring str);

		void msberr(std::string errstr);
		void msberr();

		void msbinfo(std::string errstr);
		void msbinfo();


		void msberrw(std::wstring errstr);
		void msberrw();

		void msbinfow(std::wstring infostr);
		void msbinfow();

		template<typename... Args> void pr(Args... args) { std::wstring wstr = stru::compose(args...);	ss << wstr; }
		template<typename... Args> void prt(Args... args) { std::wstring wstr = stru::compose(args...);	rtss << wstr; }
		
		template<typename... Args> void prln(Args... args) { std::wstring wstr = stru::compose(args..., "\n");	rtss << wstr; }
		template<typename... Args> void prtln(Args... args) { std::wstring wstr = stru::compose(args..., "\n");	rtss << wstr; }

		std::wstring tows(const wchar_t* c);
		std::wstring tows(const char* c);
		std::wstring tows(std::string const& str);

		std::u32string toutf32(const std::string& s);

		std::string tos(std::wstring wstr);

		void mbe(std::wstring const& errstr);
		void mbe(std::string const& errstr);
		template<typename... Args> void mbe(Args... args) { std::wstring wstr = stru::compose(args...);	mbe(wstr); }
		void mbe();

		void mbi(std::wstring const& errstr);
		void mbi(std::string const& errstr);
		template<typename... Args> void mbi(Args... args) { std::wstring wstr = stru::compose(args...);	mbi(wstr); }
		void mbi();


		void outStr(std::wstring const& str);
		template<typename... Args> void outStr(Args... args) { std::wstring wstr = stru::compose(args...);	outStr(wstr); }
		void outStr();
	}

	namespace deb = debug;
}
