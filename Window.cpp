#include "stdafx.h"

#include "resource.h"
#include "TextFont.h"

#include "Subtitle.h"
#include "Subtitles.h"
#include "Window.h"
#include "SearchBox.h"
#include "KeyBox.h"

Window* wndInstance;

Window::Window(HINSTANCE& hInstance) :
	fontName(L"Arial"),
	informationFont(L"Arial", 14, FontStyleItalic),
	subtitles(*this)
{
	wndInstance = this;

	inDrawing = false;

	hInst = hInstance;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, 100);
	LoadString(hInstance, IDC_ULTIMATESUBTITLES, szWindowClass, 100);

	// Style with and without border
	showedStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	hidedStyle = WS_POPUP | WS_VISIBLE;

	loadProperties();

	font = new TextFont(L"Arial", 22, FontStyleBold);
	information = L"";

	if (!registerClass()) return;
	if (!createWindow()) return;

	hMenu = GetMenu(hWnd);
}

HINSTANCE Window::getInstance()
{
	return hInst;
}

HWND Window::getWnd()
{
	return hWnd;
}


#define DEFAULT_VAL(KEY, VAL) \
	if (!properties.get_child_optional(KEY)) \
	{\
		properties.put(KEY, VAL);\
	}

// Load (create with default value if not exist) the properties in the json file
void Window::loadProperties() 
{
	std::ifstream file("properties.json");
	if (file.good()) {
		try {
			boost::property_tree::read_json("properties.json", properties);
		} catch(...) {
			print("Unable to read properties.json");
		}
	}

	// Properties related to opensubtitles
	DEFAULT_VAL("opensubtitles.api", "api.opensubtitles.org");
	DEFAULT_VAL("opensubtitles.username", "");
	DEFAULT_VAL("opensubtitles.password", "");
	DEFAULT_VAL("opensubtitles.useragent", "OS Test User Agent");

	// Search
	DEFAULT_VAL("search.movie", "");
	DEFAULT_VAL("search.imdb", "");
	DEFAULT_VAL("search.language", "eng");
	DEFAULT_VAL("search.season", "");
	DEFAULT_VAL("search.episode", "");

	// Hotkeys
	DEFAULT_VAL("hotkeys.pause.enabled", true);
	DEFAULT_VAL("hotkeys.pause.keycode", VK_F2);
	DEFAULT_VAL("hotkeys.previous.enabled", true);
	DEFAULT_VAL("hotkeys.previous.keycode", VK_F1);
	DEFAULT_VAL("hotkeys.next.enabled", true);
	DEFAULT_VAL("hotkeys.next.keycode", VK_F3);
	DEFAULT_VAL("hotkeys.decrease.enabled", true);
	DEFAULT_VAL("hotkeys.decrease.keycode", VK_F4);
	DEFAULT_VAL("hotkeys.increase.enabled", true);
	DEFAULT_VAL("hotkeys.increase.keycode", VK_F5);
}

boost::property_tree::ptree& Window::getProperties()
{
	return properties;
}

void Window::saveProperties() 
{
	boost::property_tree::write_json("properties.json", properties);
}

/**
	Register the class used by the window
**/
BOOL Window::registerClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ULTIMATESUBTITLES));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ULTIMATESUBTITLES);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

/**
	Create and show the window
**/
bool Window::createWindow()
{
   hWnd = CreateWindowEx(WS_EX_LAYERED, szWindowClass, szTitle, showedStyle,
      CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInst, NULL);

   if (!hWnd) return FALSE;

   SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);

   SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

   POINT clientPt = {0, 0};
   ClientToScreen(hWnd, &clientPt);

   RECT wndRect;
   GetWindowRect(hWnd, &wndRect);

   topMargin = clientPt.y - wndRect.top;
   leftMargin = clientPt.x - wndRect.left;

   // Timer that check if the mouse is over the window or not
   SetTimer(hWnd, NULL, 100, NULL);

   // Register the hotkeys
   updateHotkey();

   show();

   UpdateWindow(hWnd);

   GetClientRect(hWnd, &clientRect);

   return true;
}

void Window::updateHotkey()
{
	// Unregister if already registered
	UnregisterHotKey(hWnd, 50);
	UnregisterHotKey(hWnd, 51);
	UnregisterHotKey(hWnd, 52);
	UnregisterHotKey(hWnd, 53);
	UnregisterHotKey(hWnd, 54);

	// Register hotkeys with the keycode in the properties
	int key;
	if (properties.get("hotkeys.pause.enabled", false)) {
		key = properties.get("hotkeys.pause.keycode", 0);
		RegisterHotKey(hWnd, 50, HIBYTE(key), LOBYTE(key));
	}
	
	if (properties.get("hotkeys.previous.enabled", false)) {
		key = properties.get("hotkeys.previous.keycode", 0);
		RegisterHotKey(hWnd, 51, HIBYTE(key), LOBYTE(key));
	}

	if (properties.get("hotkeys.next.enabled", false)) {
		key = properties.get("hotkeys.next.keycode", 0);
		RegisterHotKey(hWnd, 52, HIBYTE(key), LOBYTE(key));
	}

	if (properties.get("hotkeys.decrease.enabled", false)) {
		key = properties.get("hotkeys.decrease.keycode", 0);
		RegisterHotKey(hWnd, 53, HIBYTE(key), LOBYTE(key));
	}

	if (properties.get("hotkeys.increase.enabled", false)) {
		key = properties.get("hotkeys.increase.keycode", 0);
		RegisterHotKey(hWnd, 54, HIBYTE(key), LOBYTE(key));
	}
}

Subtitles& Window::getSubtitles()
{
	return subtitles;
}

void Window::show()
{
	showed = true;

	// Reset the ex-style
	SetWindowLong(hWnd, GWL_EXSTYLE, NULL);

	// Add the border of the window
	SetWindowLong(hWnd, GWL_STYLE, showedStyle);

	// Add the menu
	SetMenu(hWnd, hMenu);
}

void Window::hide()
{
	showed = false;

	// Reset the ex-style
	SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED);

	// Remove the border of the window
	SetWindowLong(hWnd, GWL_STYLE, hidedStyle);

	// Hide the menu
	SetMenu(hWnd, NULL);

	// Make the white color trensparent
	updateText();
}

/**
	Loop that handle messages received by the window
**/
int Window::messageLoop()
{
	// Load keyboard shortcuts
	HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_ULTIMATESUBTITLES));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

void Window::drawHbitmap(HBITMAP bitmap)
{
    HDC hdc = GetDC(0);
    HDC compatibleHdc = CreateCompatibleDC(hdc);
    ReleaseDC(0, hdc);

    HBITMAP hbmpold = (HBITMAP)SelectObject(hdc, bitmap);

    POINT dcOffset = {0, 0};
    SIZE size = {640, 480};
    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;
    int result = UpdateLayeredWindow(hWnd, GetDC(NULL), NULL, NULL, compatibleHdc, NULL, RGB(0,0,0), &bf, ULW_ALPHA);

    SelectObject(compatibleHdc, hbmpold);
    DeleteDC(compatibleHdc);
    DeleteObject(bitmap);
}

void Window::updateText()
{
	CRect rectClient;
    GetClientRect(hWnd, &rectClient);

    CImage img;
    if(img.Create(rectClient.Width(), rectClient.Height(), 32, CImage::createAlphaChannel))
    {
        drawText(img.GetDC());
		img.ReleaseDC();

        POINT pt = {0, 0};
        SIZE size = {rectClient.Width(), rectClient.Height()};

        BLENDFUNCTION bf = {0};
        bf.BlendOp = AC_SRC_OVER; 
        bf.BlendFlags = 0; 
        bf.AlphaFormat = AC_SRC_ALPHA;
        bf.SourceConstantAlpha = 255;

        UpdateLayeredWindow(hWnd, 0, 0, &size, img.GetDC(), &pt, 0, &bf, ULW_ALPHA);
        img.ReleaseDC();
	}
}

int Window::drawLine(HDC hdc, int oy, std::wstring string, TextFont* fnt)
{
	using namespace Gdiplus;
	int x = (showed ? 0 : leftMargin) - 5;
	int y = (showed ? 0 : topMargin) + oy;
	const wchar_t* text = string.c_str();
	if (fnt == 0) fnt = font;

	Graphics graphics(hdc);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	FontFamily fontFamily;
	fnt->GetFamily(&fontFamily);

	RectF boundRect = fnt->getTextBounds(hdc, clientRect, text);

	x += ((clientRect.right - clientRect.left) - (int)(boundRect.GetRight() - boundRect.GetLeft()) - 2) / 2;
	y -= ((int)boundRect.GetBottom()) + 5;
	
	StringFormat strformat;

	GraphicsPath path;
	path.AddString(text, wcslen(text), &fontFamily, 
		fnt->GetStyle(), graphics.GetDpiY() * fnt->GetSize() / 72, Gdiplus::Point(x, y), &strformat );
	
	// Outline color + size
	Pen pen(Color(0, 0, 0), fnt->GetSize()/7);
	pen.SetLineJoin(LineJoinRound);
	graphics.DrawPath(&pen, &path);

	// Text color
	SolidBrush brush(Color(254, 254, 254));
	graphics.FillPath(&brush, &path);

	Rect bounds;
	path.GetBounds(&bounds, 0, &pen);

	return (int)boundRect.GetBottom();
}

void Window::drawText(HDC hdc)
{
	inDrawing = true;
	// Draw lines of subtitles
	int y = clientRect.bottom;
	// Copy the vector to avoid concurrent modifications
	if (lines.size() > 0) {
		std::vector<std::wstring> tmp(lines);

		for(std::vector<std::wstring>::reverse_iterator it = tmp.rbegin(); it != tmp.rend(); ++it) {
			y -= drawLine(hdc, y, *it);
		}
	}	

	// Draw informations (like if the player is paused...)
	if (!information.empty()) drawLine(hdc, y, information, &informationFont);

	inDrawing = false;
}



void Window::onPaint(HDC hdc) 
{
	if (showed) drawText(hdc);
}

void Window::updateSubtitles(std::vector<std::wstring>& newLines) 
{
	if (inDrawing) return;
	lines = newLines;
	if (showed) {
		InvalidateRect(hWnd, NULL, TRUE);
	} else {
		updateText();
	}
}

void Window::adjust(wchar_t* largestLine, int height)
{
	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof (MONITORINFO);
	GetMonitorInfo(hMon , &monInfo);

	RectF bounds = font->getTextBounds(GetDC(NULL), monInfo.rcMonitor, largestLine);
	
	RECT rect = {0, 0, (LONG)(bounds.GetRight()) + 60, (LONG)(bounds.GetBottom()) * height + 10};
	clientRect.top = rect.top;
	clientRect.left = rect.left;
	clientRect.bottom = rect.bottom;
	clientRect.right = rect.right;
	AdjustWindowRect(&rect, showedStyle, TRUE);
	int x = ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) - (rect.right - rect.left)) / 2;
	int y = (monInfo.rcMonitor.bottom - monInfo.rcMonitor.top) - (rect.bottom - rect.top);
	MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

void Window::setInformation(std::wstring info)
{
		information = info;
		updateSubtitles(lines);
}

LRESULT Window::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_FILE_SEARCHSUBTITLE:
		{
			SearchBox sb(*wndInstance);
			sb.show(hWnd);
			break;
		}
		case IDM_ABOUT:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, aboutProc);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_OPTIONS_HOTKEYS:
			KeyBox(*this);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		onPaint(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		if (!subtitles.isPaused())
		{
			RECT rc;
			POINT pt;
			GetWindowRect(hWnd,&rc);
			GetCursorPos(&pt);
			if (PtInRect(&rc,pt)) {
				if (!showed) show();
			} else {
				if (showed) hide();
			}
		} else if (!showed) show();
		break;
	case WM_SIZE:
		if (showed) GetClientRect(hWnd, &clientRect);
		break;
	case WM_HOTKEY:
		if (wParam == 51) subtitles.goTo(subtitles.getPreviousSubtitle());
		if (wParam == 50) subtitles.pause();
		if (wParam == 52) subtitles.goTo(subtitles.getNextSubtitle());
		if (wParam == 53) subtitles.addDelay(-50);
		if (wParam == 54) subtitles.addDelay(50);
		break;
	case WM_MOUSEWHEEL:
		{
			Gdiplus::REAL size = font->GetSize();
			size += (HIWORD(wParam) <= WHEEL_DELTA) ? 1 : -1;
			Gdiplus::FontStyle style = (Gdiplus::FontStyle)font->GetStyle();
			FontFamily fontFamily;
			font->GetFamily(&fontFamily);
			WCHAR name[LF_FACESIZE];
			fontFamily.GetFamilyName(name);
			delete font;
			font = new TextFont(name, size, style);
			subtitles.adjustWindow();
			updateSubtitles(lines);
			break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK Window::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* wnd = (Window*) GetWindowLong(hWnd, GWL_USERDATA);
	if (wnd) return wnd->proc(hWnd, message, wParam, lParam);

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK Window::aboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

Window::~Window(void)
{
	saveProperties();
	delete font;
}
