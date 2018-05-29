//#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
//#include <Windows.h>
#include <math.h>
#include <crtdbg.h>


#include "Engine/Application/Window.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Renderer/Renderer.hpp"


#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/Engine.hpp"

#define WM_CLOSE                        0x0010
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101

void WindowsMessageHandlingProcedure(uint wmMessageCode, size_t /*wParam*/, size_t lParam) {
  UNUSED(lParam);
	switch (wmMessageCode) {
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:
	{
		g_theApp->setQuitFlag(1);
		return; // "Consumes" this message (tells Windows "okay, we handled it")
	}
	}
}



//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
//
void RunFrame() {
	g_theApp->beforeFrame();
	g_theApp->update();
	g_theApp->render();
	g_theApp->afterFrame();
}

//-----------------------------------------------------------------------------------------------
void Initialize() {
  Engine::Get();
  g_theWindow = Window::Get();
  g_theWindow->addWinMessageHandler(WindowsMessageHandlingProcedure);

  g_theRenderer = Renderer::Get();
  g_theApp = new App();
}


//-----------------------------------------------------------------------------------------------
void Shutdown() {
	// Destroy the global App instance
	delete g_theApp;
	g_theApp = nullptr;
}


//-----------------------------------------------------------------------------------------------
int __stdcall WinMain(void*, void*, const char* commandLineString, int) {
	UNUSED(commandLineString);
	Initialize();

	// Program main loop; keep running frames until it's time to quit
	while (!g_theApp->isQuitting())
	{
		RunFrame();
	}

	Shutdown();
	return 0;
}


