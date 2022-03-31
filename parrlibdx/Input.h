#pragma once

#include <windows.h>

#include <parrlib/math/vector2f.h>
#include <parrlib/math/vector3f.h>
#include <parrlib/math/utils2d/axisalignedboundingbox2d.h>

namespace prb {
	namespace DXVideoPlayer { class Sample3DSceneRenderer; }

	namespace Input {

		extern int mWheel;

		void init(HWND window);

		void update();

		bool getKey(int key);
		bool getKeyDown(int key);
		bool getKeyDown(int key);

		vec2 getRawMousePos();
		vec2 getRawMouseDelta();
		vec2 getMousePos();
		vec2 getMouseDelta();
		int getMWheel();
		bool getMouse(int button);
		bool getMouseDown(int button);
		bool getMouseUp(int button);

		bool leftMouse();   //alias for input::getMouse(0)
		bool rightMouse();  //alias for input::getMouse(1)
		bool middleMouse(); //alias for input::getMouse(2)
		bool mouse();       //alias for input::getMouse(0)

		bool leftClick();	//alias for input::getMouseUp(0)
		bool rightClick();	//alias for input::getMouseUp(1)
		bool middleClick(); //alias for input::getMouseUp(2)
		bool click();		//alias for input::getMouseUp(0)

		bool leftClickDown();	//alias for input::getMouseDown(0)
		bool rightClickDown();	//alias for input::getMouseDown(1)
		bool middleClickDown(); //alias for input::getMouseDown(2)
		bool clickDown();		//alias for input::getMouseDown(0)

		vec2 getIAxis2();
		vec3 getIAxis3();

		bool getAnyButtonMouseOrKeyboard();
		bool getAnyInputMouseOrKeyboard();

		std::vector<int> getPressedKeys();
		std::vector<int> getPressedKeysDown();
		std::vector<int> getPressedKeysUp();

		int returnAnyButtonMouseOrKeyboard();
		int returnAnyInputMouseOrKeyboard();

		bool mouseInQuad(aabb2 const& bb);
		bool mouseInQuad(vec2 const& pos, vec2 const& size);

		aabb2 getClientRect();
		aabb2 getWindowRect();
	}

#ifndef PARRLIBDX_INPUT_H_SHORT
#define PARRLIBDX_INPUT_H_SHORT
	namespace input = Input;
#endif
}
