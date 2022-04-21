#pragma once

#include "../ui/imui.h"
#include "console.h"

namespace prb {
	namespace debug { struct fParam; class console; }
	namespace debugmenu {
		extern bool enabled;

		void init();
		void update();
		void dispose();

		extern debug::console cons;
		void addConsoleCommand(std::wstring name, std::wstring desc, std::vector<debug::fParam> params, std::function<std::wstring(std::vector<std::wstring>)> func);
		void addConsoleCommand(std::wstring pattern, std::wstring desc, std::function<std::wstring(std::vector<std::wstring>)> func);
	}

#ifndef PARRLIB_NAMESPACE_SHORTEN
	namespace dbm = debugmenu;
#endif
}
