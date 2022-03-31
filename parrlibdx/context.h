#pragma once

// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

#include <functional>
#include <vector>
#include <chrono>
#include <thread>
#include <unordered_map>

#include <parrlib/math/vector2f.h>
#include <parrlib/math/vector4f.h>
#include <parrlib/math/matrix3f.h>
#include <parrlib/constants.h>

namespace prb {
	/*
		Main DirectX11 context
		funcs description:
		funcs[0 | FINIT] = init	--run once at the start
		funcs[1 | FUPDATE] = update --run every frame
		funcs[2 | FDRAW] = draw	--optional, run every frame after update()
		funcs[3 | FDESTROY] = destroy --run once when the application is closed (before DirectX11 destruction)
		funcs[4 | FRESIZE] = resize --run every frame while the app is being resized
		funcs[5 | FSTARTRESIZE] = startresize --run once when the app starts being resized
		funcs[6 | FENDRESIZE] = endresize --run once when the app stops being resized
		funcs[7 | FPREUPDATE] = preupdate --run every frame before starting the new frame (before clearing the current frame)
	*/
	namespace DXContext {
		extern const int FINIT;
		extern const int FUPDATE;
		extern const int FDRAW;
		extern const int FDESTROY;
		extern const int FRESIZE;
		extern const int FSTARTRESIZE;
		extern const int FENDRESIZE;
		extern const int FPREUPDATE;
		extern std::unordered_map<unsigned int, std::function<void()>> funcs;

		extern std::function<void(HWND, UINT, WPARAM, LPARAM)> fWindowProcPrec;
		extern std::function<void(HWND, UINT, WPARAM, LPARAM)> fWindowProcLate;

		void setPeekMessageAction(UINT val);
		UINT getPeekMessageAction();

		void setFinished(bool finished);
		bool getFinished();

		void setHandleMessageLoop(bool handleMessageLoop);
		bool getHandleMessageLoop();

		void setMaximized(bool maximized);
		bool getMaximized();

		void setWindowPosition(vec2 wPos);
		vec2 getWindowPosition();

		//void setFullscreen(bool fullscreen);
		//bool getFullscreen();

		void setTitle(std::wstring title);
		std::wstring getTitle();

		void setClearColor(vec4 color);
		vec4 getClearColor();

		double getCurTime();

		//skips the next frame, it is advised to call this function in FPREUPDATE
		void skipFrame();

		void setFramerateCap(int framerateCap);
		int getFramerateCap();

		int getMaxRefreshRate();

		bool isResizing();

		class DeviceMode {
		public:
			vec2 res; int refreshRate;
			DEVMODE rawMode;
			DeviceMode(vec2 res, int refreshRate, DEVMODE rawMode) : res(res), refreshRate(refreshRate), rawMode(rawMode) {}
		};
		extern std::vector<DeviceMode> vmodes;

		/*
			Initializes the main DirectX11 context and starts running the application
			See DXContext comments for funcs descriptions
		*/
		void setup(HINSTANCE hInstance);
		void setup(vec2 res, HINSTANCE hInstance);

		/*
			Initializes the main DirectX11 context and starts running the application
			funcs should be passed in this order (you can pass NULL or nullptr to skip functions you don't want to implement):
			init, update, draw, destroy, resize, startresize, endresize, preupdate
		*/
		void setup(HINSTANCE hInstance, std::vector<std::function<void()>> const& tfuncs);

		void setup(vec2 res, HINSTANCE hInstance, std::vector<std::function<void()>> const& tfuncs);

		////returns the resolution of the native window
		//vec2 wres();
		////returns the resolution of the rendering framebuffer
		//vec2 res();

		//void resize(vec2 const& size);

		void setVSync(bool vSync);
		bool getVSync();

		void setAntiAliasing(int aa);
		int getAntiAliasing();

		//void setClearColor(vec4 col);
		//vec4 getClearColor();

		vec2 res();

		mat3 getAspectOrthoX();
		mat3 getAspectOrthoY();
		mat3 getMinAspectOrtho();
		mat3 getMaxAspectOrtho();
		mat3 getAspectOrtho();

		mat3 getAspectOrthoXInv();
		mat3 getAspectOrthoYInv();
		mat3 getMinAspectOrthoInv();
		mat3 getMaxAspectOrthoInv();
		mat3 getAspectOrthoInv();

		mat3 getScreenNormalSpace();   //alias for getMaxAspectOrthoInv

		aabb2 getScreenBoundingBox();
	}

	namespace prc = DXContext;
}
