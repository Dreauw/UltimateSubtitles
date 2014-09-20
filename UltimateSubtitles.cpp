// UltimateSubtitles.cpp : Defines the entry point for the application.
//


#include "stdafx.h"

#include "TextFont.h"
#include "Subtitle.h"
#include "Subtitles.h"
#include "UltimateSubtitles.h"
#include "Window.h"
#include "SearchBox.h"

// Enable visual style
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment (lib, "shlwapi.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "zdll.lib")
#pragma comment(lib, "wininet")


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	CoInitialize (NULL);

	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
    ULONG_PTR m_gdiplusToken;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
	

	Window* wnd = new Window(hInstance);
	wnd->setInformation(L"(No subtitles loaded)");
	wnd->messageLoop();


	delete wnd;
	
	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	CoUninitialize();


}