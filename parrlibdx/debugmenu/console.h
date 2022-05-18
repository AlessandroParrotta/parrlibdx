#pragma once

#include "../textrenderer.h"

namespace prb {
	namespace debug {
		struct fParam {
			std::wstring name, desc;

			operator std::wstring() const { return name; }
		};

		class console {
		protected:
			vec2 vpos, vsize;
			mat3 vtr; mat3 vtri;
			void calcTr();
			int off = 0;

			std::vector<std::wstring> strs;
			std::wstring text;

			void helloString();

		public:
			TextRenderer txr;

			struct command {
				std::wstring desc;
				std::function<std::wstring(std::vector<std::wstring>)> func;
				std::vector<fParam> params;
			};

			// <commandName, <func, funcParams>>
			std::unordered_map<std::wstring, command> commands;

			std::vector<std::wstring> comHistory;
			int curComHistory = 0;

			void listCommand(std::wstring name);
			void addCommand(std::wstring name, std::wstring desc, std::vector<fParam> params, std::function<std::wstring(std::vector<std::wstring>)> func);
			void addCommand(std::wstring pattern, std::wstring desc, std::function<std::wstring(std::vector<std::wstring>)> func);

			void addStartupCommands();

			vec2 pos() const;  void pos(vec2 pos);
			vec2 size() const; void size(vec2 size);
			mat3 tr() const;

			console();
			void init(vec2 const& pos, vec2 const& size);
			void init(aabb2 const& bb);

			template<typename... Args> inline void prw(Args... args) {
				std::wstring str = stru::fmtw(args...);
				if (str.length() == 0) return;
				
				str = stru::replace(str, L"\t", L"    ");

				std::vector<std::wstring> lines = stru::split(str, L"\n");
				if (strs.size() <= 0) strs.push_back(L""); if (lines.size() > 0) strs.back() += lines[0];
				for (int i = 1; i < lines.size(); i++) strs.push_back(lines[i]);
			}

			void exec(std::wstring const& line);

			void update();
			void draw();
		};
	}
}
