#include "context.h"

#include <windows.h>

#include <parrlibcore/timer.h>
#include <parrlibcore/otherutil.h>
#include <parrlibcore/tick.h>

#include "common.h"
#include "Input.h"
#include "debug.h"
#include "shader.h"
#include "util.h"
#include "debugmenu\debugmenu.h"

namespace prb {
    namespace Context {
        const int F_SIZE = 8;

        const int FINIT = 0;
        const int FUPDATE = 1;
        const int FDRAW = 2;
        const int FDESTROY = 3;
        const int FRESIZE = 4;
        const int FSTARTRESIZE = 5;
        const int FENDRESIZE = 6;
        const int FPREUPDATE = 7;
        std::unordered_map<unsigned int, std::function<void()>> funcs;

        std::function<void(HWND, UINT, WPARAM, LPARAM)> fWindowProcPrec = [](HWND, UINT, WPARAM, LPARAM) {};
        std::function<void(HWND, UINT, WPARAM, LPARAM)> fWindowProcLate = [](HWND, UINT, WPARAM, LPARAM) {};

        bool inApp = false;

        vec2 wsize = 0.f;		//frontbuffer size
        aabb2 wbb = 0.f;		//frontbuffer bounding box
        aabb2 sbb = 0.f;		//logical screen bounding box

        vec2 wres() { return wsize; }

        UINT peekMessageAction = PM_REMOVE;
        void setPeekMessageAction(UINT val) { prc::peekMessageAction = val; }
        UINT getPeekMessageAction() { return peekMessageAction; }

        std::vector<DeviceMode> vmodes;
        std::vector<DeviceMode> vidmodes;

        vec2 wPos = 0.f;
        void setWindowPosition(vec2 wPos) { prc::wPos = wPos; }
        vec2 getWindowPosition() { return wPos; }

        bool maximized = false;
        void setMaximized(bool maximized) { prc::maximized = maximized; }
        bool getMaximized() { return maximized; }

        bool finished = false;
        void setFinished(bool finished) { prc::finished = finished; }
        bool getFinished() { return finished; }

        bool handleMessageLoop = true;
        void setHandleMessageLoop(bool handleMessageLoop) { prc::handleMessageLoop = handleMessageLoop; }
        bool getHandleMessageLoop() { return handleMessageLoop; }

        double getCurTime() { return curtime; }

        bool initialized = false;

        std::wstring title = L"DirectX11 Parrlib Application";
        void setTitle(std::wstring title) {
            prc::title = title;
            if (initialized) {
                SetWindowTextW(windowHwnd, title.c_str());
            }
        }
        std::wstring getTitle() { return title; }

        bool fullscreen = false;
        bool vSync = false;
        bool focused = true;

        float lerpDeltaTime = 0.f;
        float lerpFps = 0.f;

        int framerateCap = 0;
        void setFramerateCap(int framerateCap) { prc::framerateCap = framerateCap; }
        int getFramerateCap() { return framerateCap; }

        std::chrono::high_resolution_clock::time_point microsNow;
        std::chrono::high_resolution_clock::time_point microsLater;

        std::chrono::high_resolution_clock::time_point deltaNow;
        std::chrono::high_resolution_clock::time_point deltaLater;

        vec4 clearColor = "#000000ff";
        void setClearColor(vec4 color) { clearColor = color; }
        vec4 getClearColor() { return clearColor; }

        void setVSync(bool vSync) { prc::vSync = vSync; }
        bool getVSync() { return vSync; }

        int antiAlias = 1;
        void setAntiAliasing(int aa) { prc::antiAlias = aa; }
        int getAntiAliasing() { return antiAlias; }

        bool skipFrameb = false;
        void skipFrame() { skipFrameb = true; }

        int maxRefreshRate = 0;
        int getMaxRefreshRate() { return maxRefreshRate; }

        bool oldResizing = false, resizing = false;
        bool isResizing() { return resizing; }

        int oldMouseCursorDebug = input::MOUSE_NORMAL;
        void RenderFrame(void)
        {
            if (resizing && !oldResizing)   funcs[FSTARTRESIZE]();
            if (resizing)                   funcs[FRESIZE]();
            if (!resizing && oldResizing)   funcs[FENDRESIZE]();

            deltaLater = std::chrono::high_resolution_clock::now();
            std::chrono::duration<uint64_t, std::nano> timeDiff = deltaLater - deltaNow;
            double renderTime = timeDiff.count();

            deltaTime = renderTime / (double)1e09;

            deltaNow = std::chrono::high_resolution_clock::now();

            curtime += deltaTime;

            tick::time = curtime;
            tick::deltaTime = deltaTime;

            lerpFps += ((1. / deltaTime) - lerpFps) * 6.f * deltaTime;
            lerpDeltaTime += (deltaTime - lerpDeltaTime) * 6.f * deltaTime;

            deb::prt("FPS: ", (int)lerpFps, " (", stru::ts::fromSec(lerpDeltaTime, 2), ")\n");
            //deb:prt("FPS: ", (1. / deltaTime), "\n");

            devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(clearColor.x, clearColor.y, clearColor.z, clearColor.w));

            //float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            UINT sampleMask = 0xffffffff;
            devcon->OMSetBlendState(g_pBlendStateNoBlend, NULL, sampleMask);
            
            input::processInput();

            if (input::getKeyDown(VK_F1)) {
                debugmenu::enabled = !debugmenu::enabled;

                if (debugmenu::enabled) {
                    oldMouseCursorDebug = input::getMouseStatus();
                    if (input::getMouseStatus() != input::MOUSE_NORMAL) input::toggleMouseStatus();
                }
                else {
                    input::setMouseStatus(oldMouseCursorDebug);
                }
            }

            sbb = { 0.f, wsize }; sbb = pmat3::getNdc(wbb) * sbb;

            util::multMatrix(util::getAspectOrtho());

            inApp = true;

            //deb::out("calling update\n");
            funcs[FUPDATE]();
            funcs[FDRAW]();

            if (!debugmenu::enabled) imui::reset();

            inApp = false;

            imui::setSpace(1.f);
            if (debugmenu::enabled) {
                debugmenu::update(); imui::reset();
            }

            deb::drawDebStrs(cst::res());

            ThrowIfFailed(swapchain->Present(vSync, 0));

            if (util::mStack.size() > 1) {
                deb::ss << "warning: degenerate matrix stack\n";
                util::mStack.clear(); util::mStack.push_back(1.f);
            }

            util::mStack.clear();
            util::mStack.push_back(1.f);

            input::scrollWheel = 0;

            oldResizing = resizing;
            resizing = false;
        }

        void InitGraphics()
        {
            // create a triangle using the VERTEX struct
            /*VERTEX OurVertices[] =
            {
                {0.f, 0.f,     D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
                {1.f, 0.f,     D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
                {1.f, 1.f,     D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)}
            };*/

            std::vector<float> verts = { //clockwise order
                 -1.f,-1.f, 1.f,1.f,1.f,1.f, 0.f,1.f,
                 -1.f,1.f, 1.f,1.f,1.f,1.f, 0.f,0.f,
                 1.f,1.f, 1.f,1.f,1.f,1.f, 1.f,0.f,

                 1.f,1.f, 1.f,1.f,1.f,1.f, 1.f,0.f,
                 1.f,-1.f, 1.f,1.f,1.f,1.f, 1.f,1.f,
                 -1.f,-1.f, 1.f,1.f,1.f,1.f, 0.f,1.f,
            };

            //swapchain->ResizeBuffers()

            // create the vertex buffer
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(bd));

            bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
            bd.ByteWidth = sizeof(float) * verts.size();             // size is the VERTEX struct * 3
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

            dev->CreateBuffer(&bd, NULL, &pVBuffer);       // create the buffer


            // copy the vertices into the buffer
            D3D11_MAPPED_SUBRESOURCE ms;
            devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
            memcpy(ms.pData, &verts[0], sizeof(float) * verts.size());          // copy the data
            devcon->Unmap(pVBuffer, NULL);                                      // unmap the buffer
        }

        void InitPipeline()
        {
            defShader = Shader("assets/shaders/defaultv.cso", "assets/shaders/defaultp.cso",
                {
                    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
                }
            );
            defShader.use();

            defTexShader = Shader("assets/shaders/texturev.cso", "assets/shaders/texturep.cso");
        }

        void InitD3D(HWND hWnd)
        {
            // create a struct to hold information about the swap chain
            DXGI_SWAP_CHAIN_DESC scd;

            // clear out the struct for use
            ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

            // fill the swap chain description struct
            scd.BufferCount = 1;                                    // one back buffer
            scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
            scd.BufferDesc.Width = cst::resx();                     // set the back buffer width
            scd.BufferDesc.Height = cst::resy();                    // set the back buffer height
            scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // how swap chain is to be used
            scd.OutputWindow = hWnd;                               // the window to be used
            scd.SampleDesc.Count = antiAlias;                              // how many multisamples
            scd.Windowed = TRUE;                                   // windowed/full-screen mode
            scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching

            // create a device, device context and swap chain using the information in the scd struct
            ThrowIfFailed(D3D11CreateDeviceAndSwapChain(NULL,
                D3D_DRIVER_TYPE_HARDWARE,
                NULL,
                D3D11_CREATE_DEVICE_DEBUG,
                NULL,
                NULL,
                D3D11_SDK_VERSION,
                &scd,
                &swapchain,
                &dev,
                NULL,
                &devcon)
            );

            // get the address of the back buffer
            ID3D11Texture2D* pBackBuffer;
            ThrowIfFailed(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));

            // use the back buffer address to create the render target
            ThrowIfFailed(dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer));
            pBackBuffer->Release();
            

            // set the render target as the back buffer
            devcon->OMSetRenderTargets(1, &backbuffer, NULL);

            // Set the viewport
            D3D11_VIEWPORT viewport;
            ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = cst::resx();
            viewport.Height = cst::resy();

            devcon->RSSetViewports(1, &viewport);

            InitPipeline();
            InitGraphics();

            D3D11_BLEND_DESC BlendState;
            ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));
            BlendState.RenderTarget[0].BlendEnable = TRUE;
            BlendState.AlphaToCoverageEnable = FALSE;
            BlendState.IndependentBlendEnable = FALSE;
            BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
            BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
            BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            ThrowIfFailed(dev->CreateBlendState(&BlendState, &g_pBlendStateNoBlend));



            ID3D11RasterizerState* state = nullptr;
            devcon->RSGetState(&state);

            D3D11_RASTERIZER_DESC rsdesc;
            if (state) state->GetDesc(&rsdesc);
            rsdesc.CullMode = D3D11_CULL_NONE;

            rsdesc.AntialiasedLineEnable = false;
            rsdesc.CullMode = D3D11_CULL_NONE;
            rsdesc.DepthBias = 0;
            rsdesc.DepthBiasClamp = 0.0f;
            rsdesc.DepthClipEnable = true;
            rsdesc.FillMode = D3D11_FILL_SOLID;
            rsdesc.FrontCounterClockwise = true;
            rsdesc.MultisampleEnable = false;
            rsdesc.ScissorEnable = false;
            rsdesc.SlopeScaledDepthBias = 0.0f;

            ID3D11RasterizerState* nState = nullptr;
            ThrowIfFailed(dev->CreateRasterizerState(&rsdesc, &nState));
            devcon->RSSetState(nState);

            if (state) state->Release();
        }

        void CleanD3D(void)
        {
            funcs[FDESTROY]();

            swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

            // close and release all existing COM objects
            //pLayout->Release();
            //pVS->Release();
            //pPS->Release();
            defShader.dispose();
            defTexShader.dispose();
            pVBuffer->Release();
            swapchain->Release();
            backbuffer->Release();
            dev->Release();
            devcon->Release();
        }

        void resize(vec2 size) {
            if (!swapchain) return;
            RECT wrect; GetWindowRect(windowHwnd, &wrect);
            wsize = vec2(wrect.right - wrect.left, wrect.bottom - wrect.top);

            wbb = { 0.f, wsize };

            RECT rect; GetClientRect(windowHwnd, &rect);
            int nSizeX = rect.right - rect.left, nSizeY = rect.bottom - rect.top;

            devcon->OMSetRenderTargets(0, 0, 0);

            backbuffer->Release();

            ThrowIfFailed(swapchain->ResizeBuffers(0, nSizeX, nSizeY, DXGI_FORMAT_UNKNOWN, D3D11_CREATE_DEVICE_DEBUG));

            // get the address of the back buffer
            ID3D11Texture2D* pBackBuffer;
            ThrowIfFailed(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));

            // use the back buffer address to create the render target
            ThrowIfFailed(dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer));
            pBackBuffer->Release();

            // set the render target as the back buffer
            devcon->OMSetRenderTargets(1, &backbuffer, NULL);


            // Set the viewport
            D3D11_VIEWPORT viewport;
            ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = nSizeX;
            viewport.Height = nSizeY;

            devcon->RSSetViewports(1, &viewport);

            AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

            cst::res(vec2(nSizeX, nSizeY));

            resizing = true;

            //funcs[FSTARTRESIZE]();
            //funcs[FRESIZE]();       //TODO: fix start resize and end resize
            //funcs[FENDRESIZE]();

            RenderFrame();
        }

        LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {

            fWindowProcPrec(hWnd, message, wParam, lParam);

            switch (message)
            {
            case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
            case WM_SIZE:
            {
                if (!swapchain) return 0;
                RECT wrect; GetWindowRect(windowHwnd, &wrect);
                wsize = vec2(wrect.right - wrect.left, wrect.bottom - wrect.top);

                RECT rect; GetClientRect(windowHwnd, &rect);
                int nSizeX = rect.right - rect.left, nSizeY = rect.bottom - rect.top;

                resize(vec2(nSizeX, nSizeY));
            }break;
            case WM_MOUSEWHEEL:
            {
                short wval = (short)(wParam >> 16);
                input::scrollWheel = (wval > 0 ? 1 : (wval < 0 ? -1 : 0));
            }break;
            case WM_PAINT:
            {
                if (curtime < .1f) {
                    PAINTSTRUCT ps;
                    HBRUSH br = CreateSolidBrush(RGB(0, 0, 0));

                    HDC hdc = BeginPaint(hWnd, &ps);
                        FillRect(hdc, &ps.rcPaint, br);
                    EndPaint(hWnd, &ps);

                    DeleteObject(br);
                }
                break;
            }
            //case WM_KEYDOWN:
            //{
            //    input::textKeys.push_back((int)wParam);
            //    break;
            //}
            case WM_CHAR:
            {
                input::textKeys.push_back((int)wParam);
                break;
            }
            }

            fWindowProcLate(hWnd, message, wParam, lParam);

            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        void getVideoModes() {
            int iMode = 0;

            DEVMODE defaultMode;
            ZeroMemory(&defaultMode, sizeof(DEVMODE));
            defaultMode.dmSize = sizeof(DEVMODE);

            while (EnumDisplaySettings(NULL, iMode, &defaultMode)) {
                deb::pr("display device ", defaultMode.dmFormName, ": ", defaultMode.dmPelsWidth, "x", defaultMode.dmPelsHeight, "@", defaultMode.dmDisplayFrequency, "Hz (", defaultMode.dmDeviceName, ")", "\n");
                vidmodes.push_back(DeviceMode(vec2(defaultMode.dmPelsWidth, defaultMode.dmPelsHeight), defaultMode.dmDisplayFrequency, defaultMode));
                
                iMode++;
            }
        }

        void setup(HINSTANCE hInstance) {
            // guarantees an approximate error of 1ms for every sleep call
            // if you don't call this, your timer functions will probably sleep for 3x the amount
            // you specify or more, you need to call a matching timeEndPeriod with the same quantity as this
            timeBeginPeriod(1);

            for (int i = 0; i < F_SIZE; i++) if (funcs.find(i) == funcs.end()) funcs[i] = [] {};

            deb::createWindow();

            //get active display modes (monitors)
            DWORD DispNum = 0;
            DISPLAY_DEVICE  DisplayDevice;
            DEVMODE defaultMode;

            // initialize DisplayDevice
            ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
            DisplayDevice.cb = sizeof(DisplayDevice);

            while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0)) {
                ZeroMemory(&defaultMode, sizeof(DEVMODE));
                defaultMode.dmSize = sizeof(DEVMODE);
                if (!EnumDisplaySettings((LPWSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode)) deb::ss << "could not display settings\n";
                else {
                    if (defaultMode.dmPelsWidth > 0 && defaultMode.dmPelsHeight > 0) {
                        deb::pr("display device ", DispNum, ": ", defaultMode.dmPelsWidth, "x", defaultMode.dmPelsHeight, "@", defaultMode.dmDisplayFrequency, "Hz (", defaultMode.dmDeviceName, ")", "\n");
                        vmodes.push_back(DeviceMode(vec2(defaultMode.dmPelsWidth, defaultMode.dmPelsHeight), defaultMode.dmDisplayFrequency, defaultMode));
                    }
                }

                ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
                DisplayDevice.cb = sizeof(DisplayDevice);
                DispNum++;
            }

            getVideoModes();

            const HWND hDesk = GetDesktopWindow();
            RECT desktop; GetWindowRect(hDesk, &desktop);
            if (cst::res() <= 0.f) cst::res(vec2(desktop.right, desktop.bottom));
            else { desktop.right = cst::resx(); desktop.bottom = cst::resy(); }

            WNDCLASSEX wc;

            ZeroMemory(&wc, sizeof(WNDCLASSEX));

            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = WindowProc;
            wc.hInstance = hInstance;
            //wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
            //wc.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.lpszClassName = L"WindowClass";

            RegisterClassEx(&wc);

            RECT wr = { 0, 0, (int)cst::resx(), (int)cst::resy() };
            AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

            windowHwnd = CreateWindowEx(NULL,
                L"WindowClass",
                title.c_str(),
                WS_OVERLAPPEDWINDOW,
                wPos.x,
                wPos.y,
                wr.right - wr.left,
                wr.bottom - wr.top,
                NULL,
                NULL,
                hInstance,
                NULL);

            RECT wrect; GetWindowRect(windowHwnd, &wrect);
            wsize = vec2(wrect.right - wrect.left, wrect.bottom - wrect.top);

            wbb = { 0.f, wsize };

            //THIS IS A GLOBAL COLOR CHANGE, EVEN OTHER PROGRAMS
            //int aElements[2] = { COLOR_WINDOW, COLOR_ACTIVECAPTION };
            //DWORD aNewColors[]{RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF)};
            //SetSysColors(2, aElements, aNewColors);

            EnumDisplayMonitors(NULL, NULL, 
                    [](HMONITOR monitor,
                        HDC device,
                        LPRECT rect,
                        LPARAM params
                    ) -> BOOL {

                LPMONITORINFO info = new MONITORINFO();
                info->cbSize = sizeof(MONITORINFO);
                GetMonitorInfo(monitor, info);
                //deb::pr("monitor:\n");
                //deb::pr(info->rcMonitor.left, ", ", info->rcMonitor.right, ", ", info->rcMonitor.top, ", ", info->rcMonitor.bottom, "\n");
                delete info;

                return TRUE;
            }, 0);

            ShowWindow(windowHwnd, maximized ? SW_MAXIMIZE : SW_NORMAL);
            UpdateWindow(windowHwnd);

            RECT rect; GetClientRect(windowHwnd, &rect);
            int nSizeX = rect.right - rect.left, nSizeY = rect.bottom - rect.top;
            cst::res(vec2(nSizeX, nSizeY));

            InitD3D(windowHwnd);

            //deb::out("inited D3D\n");

            input::addActiveLayer(INPUT_LAYER_DEFAULT);

            util::init();

            input::init(windowHwnd);
            deb::debInit();

            MSG msg;

            deltaNow = std::chrono::high_resolution_clock::now();

            maxRefreshRate = 0;
            for (int i = 0; i < vmodes.size(); i++) {
                //deb << "iterating " << vmodes[i].refreshRate << "\n";
                maxRefreshRate = (std::max)(maxRefreshRate, vmodes[i].refreshRate);
            }
            //deb << "maxref " << maxRefreshRate << "\n";
            //framerateCap = maxRefreshRate;

            initialized = true;

            imui::init();
            imui::useAtlas("assets/sprites/atlas.png");
            imui::setTxr("assets/fonts/segoeui.ttf", 32);
            imui::setTextColor(vc4::black);

            debugmenu::init();

            //deb::out("calling init\n");
            funcs[FINIT]();

            int timesRendered = 0;

            while (!finished)
            {
                if (handleMessageLoop) {
                    if (PeekMessage(&msg, NULL, 0, 0, peekMessageAction))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);

                        if (msg.message == WM_QUIT)
                            break;
                    }
                }

                if (skipFrameb) {
                    deltaLater = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<uint64_t, std::nano> timeDiff = deltaLater - deltaNow;
                    double renderTime = timeDiff.count();

                    deltaTime = renderTime / (double)1e09;

                    deltaNow = std::chrono::high_resolution_clock::now();

                    curtime += deltaTime;
                }

                if (!skipFrameb) {
                    if (framerateCap > 5)
                        microsNow = std::chrono::high_resolution_clock::now();

                    RenderFrame();

                    if (framerateCap > 5) {
                        microsLater = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<uint64_t, std::nano> timeDiff = microsLater - microsNow;

                        double microsToWait = (double)1e06 / (double)framerateCap;
                        double renderTime = timeDiff.count() / (double)1e03;

                        //Timer t;
                        outl::sleepPreciseUS(microsToWait - renderTime);
                        //t.set(); deb::rtss << "context sleep time " << t.getTime().c_str() << " (" << microsToWait << " - " << renderTime << " = " << (microsToWait - renderTime) <<  "us)\n";
                    }
                }
                else { deb::rtss.clear(); deb::rtss.str(L""); }

                input::update();

                funcs[FPREUPDATE]();
                //if (skipFrameb) { skipFrameb = false; return; }

                skipFrameb = false;
            }

            deb::dispose();

            CleanD3D();

            timeEndPeriod(1);
        }

        void setup(HINSTANCE hInstance, std::vector<std::function<void()>> const& tfuncs) {
            //funcs = tfuncs;
            //if (funcs.size() < 8) {
            //	while (funcs.size() < 8) funcs.push_back([] {});
            //}
            //for (int i = 0; i < funcs.size(); i++) { if (funcs[i] == NULL) funcs[i] = [] {}; }
            for (int i = 0; i < tfuncs.size(); i++) {
                if (tfuncs[i] == NULL && funcs.find(i) == funcs.end())  funcs[i] = [] {};
                else                                                    funcs[i] = tfuncs[i];
            }

            setup(hInstance);
        }

        void setup(vec2 res, HINSTANCE hInstance) {
            cst::res(res);
            setup(hInstance);
        }

        void setup(vec2 res, HINSTANCE hInstance, std::vector<std::function<void()>> const& tfuncs) {
            cst::res(res);
            setup(hInstance, tfuncs);
        }
        vec2 res() { return cst::res(); }

        mat3 getAspectOrthoX() { return pmat3::orthoAspectX(cst::res()); }
        mat3 getAspectOrthoY() { return pmat3::orthoAspectY(cst::res()); }
        mat3 getMinAspectOrtho() { return pmat3::orthoMinAspect(cst::res()); }
        mat3 getMaxAspectOrtho() { return pmat3::orthoMaxAspect(cst::res()); }
        mat3 getAspectOrtho() { return pmat3::orthoMaxAspect(cst::res()); }

        mat3 getAspectOrthoXInv() { return pmat3::orthoAspectXInv(cst::res()); }
        mat3 getAspectOrthoYInv() { return pmat3::orthoAspectYInv(cst::res()); }
        mat3 getMinAspectOrthoInv() { return pmat3::orthoMinAspectInv(cst::res()); }
        mat3 getMaxAspectOrthoInv() { return pmat3::orthoMaxAspectInv(cst::res()); }
        mat3 getAspectOrthoInv() { return pmat3::orthoMaxAspectInv(cst::res()); }

        mat3 getScreenNormalSpace() { return getMaxAspectOrthoInv(); }

        aabb2 getScreenBoundingBox() { return getScreenNormalSpace() * aabb2s(2.f); }
    }
}
