#include "Input.h"

#include "debug.h"

namespace prb {
	namespace Input {
		HWND windowHwnd;

		const int KEYS_LENGTH = 349;
		const int MOUSE_LENGTH = 31;

		int mWheel = 0;

		bool oldkeys[KEYS_LENGTH];
		bool keys[KEYS_LENGTH];

		bool mkeys[MOUSE_LENGTH];
		bool oldmkeys[MOUSE_LENGTH];

		vec2 moldpos;
		vec2 mpos;
		vec2 mdelta;


		void init(HWND hwnd) {
			windowHwnd = hwnd;
			for (int i = 0; i < KEYS_LENGTH; i++) {
				keys[i] = false;
			}

			for (int i = 0; i < MOUSE_LENGTH; i++) {
				mkeys[i] = false;
			}
		}

		void update() {
			HWND foc = GetFocus();

			for (int i = 0; i < KEYS_LENGTH; i++) {
				oldkeys[i] = keys[i];
			}

			for (int i = 0; i < MOUSE_LENGTH; i++) oldmkeys[i] = mkeys[i];

			if (foc == windowHwnd) {
				for (int i = 0; i < 256; i++) {
					keys[i] = GetAsyncKeyState(i);
				}

				mkeys[0] = GetKeyState(VK_LBUTTON) < 0;
				mkeys[1] = GetKeyState(VK_RBUTTON) < 0;
				mkeys[2] = GetKeyState(VK_MBUTTON) < 0;
				mkeys[3] = GetKeyState(VK_XBUTTON1) < 0;
				mkeys[4] = GetKeyState(VK_XBUTTON2) < 0;
			}
			else {
				for (int i = 0; i < KEYS_LENGTH; i++) if (keys[i]) keys[i] = false;
			}

			moldpos = mpos;
			mpos = getRawMousePos();
			mdelta = mpos - moldpos;
		}

		bool getKey(int key) {
			return keys[key];
		}

		bool getKeyDown(int key) {
			return keys[key] && !oldkeys[key];
		}

		bool getKeyUp(int key) {
			return !keys[key] && oldkeys[key];
		}

		vec2 getRawMousePos() {
			POINT p;
			GetCursorPos(&p);

			RECT rec;
			GetWindowRect(windowHwnd, &rec);

			//RECT crec;
			//GetClientRect(windowHwnd, &crec);

			return { (float)p.x - (float)rec.left - 8, (float)p.y - (float)rec.top - 31 };
		}

		vec2 getRawMouseDelta() {
			return mdelta;
		}

		vec2 getMousePos() {
			return mpos / getClientRect().size() * 2.f - 1.f;
		}

		vec2 getMouseDelta() {
			return mdelta / getClientRect().size() * 2.f;
		}

		int getMWheel() { return mWheel; }

		bool getMouse(int button) { return mkeys[button]; }
		bool getMouseDown(int button) { return mkeys[button] && !oldmkeys[button]; }
		bool getMouseUp(int button) { return !mkeys[button] && oldmkeys[button]; }


		bool leftMouse() { return getMouse(0); }
		bool rightMouse() { return getMouse(1); }
		bool middleMouse() { return getMouse(2); }
		bool mouse() { return getMouse(0); }

		bool leftClick() { return getMouseUp(0); }
		bool rightClick() { return getMouseUp(1); }
		bool middleClick() { return getMouseUp(2); }
		bool click() { return getMouseUp(0); }

		bool leftClickDown() { return getMouseDown(0); }
		bool rightClickDown() { return getMouseDown(1); }
		bool middleClickDown() { return getMouseDown(2); }
		bool clickDown() { return getMouseDown(0); }

		vec2 getIAxis2() {
			return vec2(
				getKey('D') - getKey('A'),
				getKey('W') - getKey('S')
			);
		}

		vec3 getIAxis3() {
			return vec3(
				getKey('D') - getKey('A'),
				getKey(VK_SPACE) - getKey('C'),
				getKey('W') - getKey('S')
			);
		}

		bool getAnyButtonMouseOrKeyboard() {
			for (int i = 1; i < KEYS_LENGTH; i++) if (keys[i])return true;
			for (int i = 1; i < MOUSE_LENGTH; i++) if (mkeys[i]) return true;
			if (mWheel != 0) return true;
			return false;
		}

		bool getAnyInputMouseOrKeyboard() {
			for (int i = 1; i < KEYS_LENGTH; i++) if (keys[i]) return true;
			for (int i = 1; i < MOUSE_LENGTH; i++) if (mkeys[i]) return true;
			if (mdelta.abs().magnitude() > 0.f) return true;
			if (mWheel != 0) return true;
			return false;
		}

		int returnAnyButtonMouseOrKeyboard() {
			for (int i = 1; i < KEYS_LENGTH; i++) if (keys[i]) return i;
			for (int i = 1; i < MOUSE_LENGTH; i++) if (mkeys[i]) return KEYS_LENGTH + i;
			if (mWheel != 0) return -2;
			return 0;
		}

		int returnAnyInputMouseOrKeyboard() {
			for (int i = 1; i < KEYS_LENGTH; i++) if (keys[i]) return i;
			for (int i = 1; i < MOUSE_LENGTH; i++) if (mkeys[i]) return KEYS_LENGTH + i;
			if (mdelta.abs().magnitude() > 0.f) return -1;
			if (mWheel != 0) return -2;
			return 0;
		}

		std::vector<int> getPressedKeys() {
			std::vector<int> keys;
			for (int i = 1; i < KEYS_LENGTH; i++) {
				if (getKey(i)) keys.push_back(i);

			}
			return keys;
		}

		std::vector<int> getPressedKeysDown() {
			std::vector<int> keys;
			for (int i = 1; i < KEYS_LENGTH; i++) {
				if (getKeyDown(i)) keys.push_back(i);

			}
			return keys;
		}

		std::vector<int> getPressedKeysUp() {
			std::vector<int> keys;
			for (int i = 1; i < KEYS_LENGTH; i++) {
				if (getKeyUp(i)) keys.push_back(i);

			}
			return keys;
		}

		bool mouseInQuad(aabb2 const& bb) { return bb.pointInside(getMousePos().ny()); }
		bool mouseInQuad(vec2 const& pos, vec2 const& size) { return mouseInQuad({ pos, pos + size }); }

		aabb2 getClientRect() { RECT rec; GetClientRect(windowHwnd, &rec); return { {(float)rec.left,(float)rec.top}, { (float)rec.right,(float)rec.bottom } }; }
		aabb2 getWindowRect() { RECT rec; GetWindowRect(windowHwnd, &rec); return { {(float)rec.left,(float)rec.top}, { (float)rec.right,(float)rec.bottom } }; }
	}
}
