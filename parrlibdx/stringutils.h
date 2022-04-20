#pragma once

#include <parrlibcore/stringutils.h>

namespace prb {
	namespace strup {
		inline std::wstring fallbackPath(std::wstring const& prefix, std::wstring const& path) { return stru::fallbackPath(prefix, L"parrlibdxassets", path); }
		inline std::wstring fallbackPath(std::wstring const& path) { return stru::fallbackPath(L"", L"parrlibdxassets", path); }
		
		inline std::string fallbackPath(std::string const& prefix, std::string const& path) { return stru::fallbackPath(prefix, "parrlibdxassets", path); }
		inline std::string fallbackPath(std::string const& path) { return stru::fallbackPath("", "parrlibdxassets", path); }
	}
}
