#include "console.h"

#include "../context.h"
#include "../input.h"
#include "../ui/imui.h"

namespace prb {
	namespace debug {

		console::console() {

		}

		void console::helloString() {
			std::time_t t = std::time(0);
			struct tm now;
			localtime_s(&now, &t);

			prw("<Parrlib DirectX11/C++, session started ",
				(now.tm_year + 1900), "/", (now.tm_mon + 1), "/", now.tm_mday, " ",
				now.tm_hour, ":", now.tm_min, ":", now.tm_sec, ">\n",
				"this library (Parrlib) was developed by Alessandro Parrotta (alessandro.parrott@gmail.com)\n",
				"type 'help <commandName>' to show the usage of a command, or 'list' to show all possible commands\n");

		}

		void console::calcTr() { vtr = pmat3::toNdc({ vpos, vpos + vsize }); vtri = vtr.inverted(); }
		vec2 console::pos() const { return vpos; };  void console::pos(vec2 pos) { vpos = pos; calcTr(); }
		vec2 console::size() const { return vsize; } void console::size(vec2 size) { vsize = size; calcTr(); }
		mat3 console::tr() const { return vtr; }

		void console::init(vec2 const& pos, vec2 const& size) {
			this->pos(pos); this->size(size);

			txr = TextRenderer({ "assets/fonts/segoeui.ttf" }, 25);

			addStartupCommands();

			helloString();
		}
		void console::init(aabb2 const& bb) { init(bb[0], bb.size()); }

		void console::listCommand(std::wstring name) {
			if (commands.find(name) == commands.end()) return;

			std::vector<fParam> args = commands[name].params;

			prw(name, " ");
			for (int i = 0; i < args.size(); i++) { prw("<", args[i].name, i >= args.size() - 1 ? ">" : "> "); }
			prw("\n", commands[name].desc, "\n");
			for (int i = 0; i < args.size(); i++) { prw("\tparam '", args[i].name, "': ", args[i].desc, "\n"); }
		}

		void console::addCommand(std::wstring name, std::wstring desc, std::vector<fParam> params, std::function<std::wstring(std::vector<std::wstring>)> func) { commands[name] = { desc, func, params }; }

		void console::addCommand(std::wstring pattern, std::wstring desc, std::function<std::wstring(std::vector<std::wstring>)> func) {
			std::vector<std::wstring> args = stru::toArgs(pattern, 1, -1);
			std::vector<fParam> params; for (int i = 0; i < args.size(); i++) params.push_back(fParam{ args[i], L"" });
			addCommand(stru::getArg(pattern, 0), desc, params, func);

			//addCommand(stru::getArg(pattern, 0), stru::toArgs(pattern, 1, -1), func);
		}

		void console::addStartupCommands() {

			addCommand(L"help", L"prints out these lines or shows the usage of a particular command, type 'list' to show all commands.",
				std::vector<fParam>{
					{ L"commandName", L"optional: the command to help with." }
			},
				[&](std::vector<std::wstring> args) -> std::wstring {
				//if (args.size() <= 0) { prw("please specify a command to help with\n"); listCommand(L"help"); return L""; }
				if (args.size() == 0) { listCommand(L"help"); return L""; }
				if (args.size() > 0 && commands.find(args[0]) == commands.end()) { prw("ERROR: command '", args[0], "' is not defined.\n");  return L""; }
				if (args.size() > 0) listCommand(args[0]);
				return L"";
			});

			addCommand(L"echo", L"print a message on the console",
				std::vector<fParam>{
					{ L"message", L"the message to print." }
			},
				[&](std::vector<std::wstring> args) -> std::wstring {
				if (args.size() <= 0) { prw("ERROR: must specify a message to print."); return L""; }
				for(int i=0; i<args.size(); i++) prw(stru::replace(args[i], L"\\n", L"\n"));
				return L"";
			});

			addCommand(L"list", L"displays all possible commands that can be executed on this console.", std::vector<fParam>{},
				[&](std::vector<std::wstring> args) -> std::wstring {
					for (auto& c : commands) { listCommand(c.first); }
					return L"";
				});

			addCommand(L"clear", L"clears all printed messages from the console.", std::vector<fParam>{},
				[&](std::vector<std::wstring> args) -> std::wstring {
					strs.clear();
					return L"";
				});

			addCommand(L"for", L"repeats command n times, replaces every ' counterName ' (removes the spaces) with the actual value of the counter.",
				std::vector<fParam>{
					{ L"counterName", L"the name of the counter." },
					{ L"begin", L"initial value of the counter." },
					{ L"end", L"final value of the counter." },
					{ L"inc", L"how much to increment the counter every loop." },
					{ L"commands : array", L"array of commands." },
			},
				[&](std::vector<std::wstring> args) -> std::wstring {
						if (args.size() < 5) { prw("ERROR: not enough arguments to execute 'for'.\n"); return L""; }

						//prw("args: ");
						//for (int i = 0; i < args.size(); i++) prw("args[", i, "]: '", args[i], "'\n");

						std::wstring counterName = args[0];
						int begin = stru::toi(args[1]);
						int end = stru::toi(args[2]);
						int inc = stru::toi(args[3]);

						std::wstring command = stru::fromArgs(outl::subV(args, 4, -1));

						//prw(begin, "\n", end, "\n", inc, "\n", command, "\n");

						for (int i = begin; i < end; i += inc) {
							exec(stru::replace(command, stru::fmtw(" ", counterName, " "), stru::fmtw(i)));
						}

						return L"";
					});

			addCommand(L"startinfo", L"prints the first message that appears when starting a session.", std::vector<fParam>{},
				[&](std::vector<std::wstring> args) -> std::wstring {
					helloString();
					return L"";
				});

			//addCommand(Console::Command("startinfo", "prints the first message that appears when starting a session", {},
			//	[this](std::vector<std::string> argvals) -> std::string {
			//		
			//	}));
		}

		void console::exec(std::wstring const& line) {
			if (line.length() <= 0) return;

			std::wstring name = stru::getArg(line, 0);
			if (commands.find(name) == commands.end()) { prw("ERROR: command '", name, "' not found, see 'help' for a list of all available commands.\n"); return; }

			std::vector<std::wstring> args = stru::toArgs(line, 1, -1);
			//if (args.size() >= commands[name].params.size()) commands[name].func(args);
			//else { prw("ERROR: not enough arguments to call command '", name, "'.\n"); listCommand(name); return; }

			std::wstring res = commands[name].func(args);

			if (res.length() > 0) prw(res);
		}

		void console::update() {
			if (input::getKeyDown(VK_UP)) { curComHistory++; curComHistory = outl::imin(curComHistory, comHistory.size() - 1); text = comHistory[curComHistory]; }
			if (input::getKeyDown(VK_DOWN)) { curComHistory--; curComHistory = outl::imax(curComHistory, 0); text = comHistory[curComHistory]; }
			
			if (input::getKeyDown(VK_PRIOR) || input::getMWheel() > 0) off += 1 * (input::getKey(VK_LSHIFT) ? 4 : 1);
			if (input::getKeyDown(VK_NEXT) || input::getMWheel() < 0) off -= 1 * (input::getKey(VK_LSHIFT) ? 4 : 1);;
			off = outl::imax(off, 0);
		}

		void console::draw() {
			int n = txr.getFontSize();
			float h = 1.f / (float)txr.getFontSize() + .01f;
			for (int i = outl::imin(strs.size() - 1, strs.size() - 1 - off); i >= outl::imax(strs.size() - 1 - n - off, 0); i--) {
				txr.drawWStringpx(strs[i], vec2(1.f, 0.f), aabb2s(4.f), pmat3::translate(vec2(-(prc::csize.x / prc::csize.y) + .005f, -.9f + h / 2.f + .005f) + vec2(0.f, h * (strs.size() - 1 - i - off))));
			}

			int inp = imui::textField("deb_console_text", L"", L"...", vec2(0.f, -.95f), vec2(4.f, .1f), &text);
			if (inp == 3 && text.size() > 0) {
				//deb::pr("prw ", stru::tostr(text), "\n");

				prw(text, "\n");
				comHistory.push_back(text);

				exec(text);

				text = L"";

				off = 0;
			}
		}
	}
}