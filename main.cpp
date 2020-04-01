#include <stdio.h>
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <string>
#include <time.h>

#include <GL/gl3w.h>
#include "wglext.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"

#include "discord-rpc/win32-dynamic/include/discord_rpc.h"

#include "ProggyClean.h"

static void Fatal(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    ExitProcess(1);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI
Win32WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static bool
Win32InitOpenGL(HDC dc, unsigned int frameVSyncSkipCount)
{
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int formatIndex = ChoosePixelFormat(dc, &pfd);
    if (!formatIndex)
    {
        Fatal("ERROR: ChoosePixelFormat failed!");
    }

    if (!SetPixelFormat(dc, formatIndex, &pfd))
    {
        Fatal("ERROR: SetPixelFormat failed!");
    }

    HGLRC legacyContext = wglCreateContext(dc);
    if (!legacyContext)
    {
        int error = glGetError();
        Fatal("ERROR: wglCreateContext failed with code %d", error);
    }

    if (!wglMakeCurrent(dc, legacyContext))
    {
        int error = glGetError();
        Fatal("ERROR: wglMakeCurrent failed with code %d", error);
    }

    int flags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#if DEBUG
    flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

    const int contextAttributes[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, flags,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB
        = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribsARB)
    {
        Fatal("ERROR: Failed querying entry point for wglCreateContextAttribsARB!");
    }

    HGLRC renderingContext = wglCreateContextAttribsARB(dc, 0, contextAttributes);
    if (!renderingContext)
    {
        int error = glGetError();
        Fatal("ERROR: Couldn't create rendering context! Error code is: %d", error);
    }

    BOOL res;
    res = wglMakeCurrent(dc, NULL);
    res = wglDeleteContext(legacyContext);

    if (!wglMakeCurrent(dc, renderingContext))
    {
        int error = glGetError();
        Fatal("ERROR: wglMakeCurrent failed with code %d", error);
    }
	
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(frameVSyncSkipCount);
    }

    return true;
}

std::string status;
time_t curtime = time(0);

void initDiscord() {
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers = {};
	Discord_Initialize("692808414409261136", &handlers, 1, NULL);
}

void UpdatePresence(std::string state) {
	char *statechr = &state[0];
	
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.state = statechr;

	discordPresence.largeImageKey = "icon";
	discordPresence.largeImageText = "Minecraft";
	
	discordPresence.startTimestamp = curtime;

	Discord_UpdatePresence(&discordPresence);
}

// got this from here https://noobtuts.com/cpp/compare-float-values
bool cmpf(float A, float B, float epsilon = 0.005f) {
	return (fabs(A - B) < epsilon);
}

int main(int, char**)
{
	initDiscord();
	UpdatePresence("");
	
    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_NOCLOSE;
    windowClass.lpfnWndProc = Win32WindowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.lpszClassName = "MCPresence";
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&windowClass))
        Fatal("ERROR: Couldn't register window class!");

    HWND window = CreateWindowEx(0, windowClass.lpszClassName, "MCPresence",
                                 WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE, 100, 100, 558, 355,
                                 0, 0, windowClass.hInstance, 0);

    if(!window)
        Fatal("ERROR: Couldn't create window!");

    HDC deviceContext = GetDC(window);
    if (!Win32InitOpenGL(deviceContext, 1))
        Fatal("ERROR: OpenGL initialization failed!");
    gl3wInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplOpenGL3_Init();
	io.IniFilename = NULL;
	
    ImGui::StyleColorsDark();
	
	ImFont* defaultf = io.Fonts->AddFontFromMemoryCompressedBase85TTF(ProggyClean_compressed_data_base85, 26.0f, NULL);
	ImFont* defaultfb = io.Fonts->AddFontFromMemoryCompressedBase85TTF(ProggyClean_compressed_data_base85, 65.0f, NULL);
    ImFont* comicf = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\comic.ttf", 28.0f, NULL);
	ImFont* comicfb = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\comic.ttf", 67.0f, NULL);
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
	// This was a console program at first, and when I sent it to my friend Bram he was like "i expected a shitty ui with comic sans and flashing rgb", so here you go Bram.
	bool BRAMMode = false;
	bool runonce = false;
	bool redzero = false;
	bool greenzero = false;
	bool bluezero = false;
	bool resetzero = false;
	float step = 0.05f;
	
	char state[128] = "";
	char statemsg[129] = "";
	
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        RECT clientRect;
        GetClientRect(window, &clientRect);
        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;

        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
		
		ImGuiStyle& style = ImGui::GetStyle();
		
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 0.0f;
		style.WindowPadding.x = 3.0f;
		style.WindowPadding.y = 3.0f;
		
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
		ImGui::Begin("MCPresence", 0 , ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);

		if (BRAMMode) {
			ImGui::PushFont(comicfb);
			
			if (!runonce) {
				clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
				runonce = true;
			}
			
			if (resetzero) {
				redzero = false;
				greenzero = false;
				bluezero = false;
				
				resetzero = false;
			}				
			
			if (!cmpf(clear_color.x, 1.0f) && !redzero) {
				clear_color.x = 1.0f;
			} else {
				if (!cmpf(clear_color.y, 1.0f) && !greenzero) {
					clear_color.y += step;
				} else {
					if (!cmpf(clear_color.x, 0.0f) && cmpf(clear_color.y, 1.0f)) {
						redzero = true;
						clear_color.x -= step;
					} else {
						if (!cmpf(clear_color.z, 1.0f) && !bluezero) {
							clear_color.z += step;
						} else {
							if (!cmpf(clear_color.y, 0.0f)) {
								greenzero = true;
								clear_color.y -= step;
							} else {
								if (!cmpf(clear_color.x, 1.0f)) {
									clear_color.x += step;
								} else {
									if (!cmpf(clear_color.z, 0.0f)) {
										bluezero = true;
										clear_color.z -= step;
									} else {
										resetzero = true;
									}
								}
							}
						}
					}
				}
			}
		} else {
			ImGui::PushFont(defaultfb);
			clear_color = ImVec4(0.14f, 0.14f, 0.14f, 0.00f);
			runonce = false;
		}
		
		const char* text = "MCPresence";		
		ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(text).x) / 2);
		ImGui::Text("%s", text);
		
		ImGui::PopFont();
		
		if (!BRAMMode) {
			ImGui::PushFont(defaultf);
		} else {
			ImGui::PushFont(comicf);
		}
		
        ImGui::InputTextWithHint("", "Enter custom status here!", state, IM_ARRAYSIZE(state));
		ImGui::SameLine();
		if (ImGui::Button("Set status"))
			strcpy(statemsg, state);
			UpdatePresence(statemsg);
			if (strcmp(state, statemsg) == 0)
				memset(state, 0, 128);
		
		ImGui::Text("Current status message: ");
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + windowWidth);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), statemsg);
		ImGui::PopTextWrapPos();
		
		/* if (ImGui::Button("tmp")) {
		}
		char buffer[64];
		int ret = snprintf(buffer, sizeof buffer, "%f", ImGui::GetItemRectSize().x);
		ImGui::Text(buffer); */
		
		ImGui::SetCursorPosY((windowHeight - ImGui::GetFrameHeight()) - style.WindowPadding.y);
		ImGui::Checkbox("BRAM Mode", &BRAMMode);
		ImGui::SameLine();
		if (BRAMMode) {
			ImGui::SetCursorPosX(windowWidth - (48 + style.WindowPadding.x));
		} else {
			ImGui::SetCursorPosX(windowWidth - (64 + style.WindowPadding.x));
		}
		
		if (ImGui::Button("Exit")) {
			break;
		}

		ImGui::PopFont();
		
		ImGui::End();

        ImGui::Render();
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(deviceContext);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(window);
    UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	
	Discord_Shutdown();
	
    return 0;
}
