#pragma once
using namespace Gdiplus;

class Window
{
public:
	Window(HINSTANCE& hInstance);
	~Window(void);
	void show();
	void hide();
	int messageLoop();
	void updateSubtitles(std::vector<std::wstring>& newLines);
	void adjust(wchar_t* largestLine, int height);
	Subtitles& getSubtitles();
	void updateHotkey();
	void setInformation(std::wstring info);
	boost::property_tree::ptree& getProperties();
	HINSTANCE getInstance();
	HWND getWnd();

private:
	BOOL registerClass();
	bool createWindow();
	void drawHbitmap(HBITMAP bitmap);
	LRESULT proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK aboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void onPaint(HDC hdc);
	void drawText(HDC hdc);
	void updateText();
	int drawLine(HDC hdc, int oy, std::wstring string, TextFont* fnt = 0);
	void loadProperties();
	void saveProperties();

	HINSTANCE hInst;
	TCHAR szTitle[100];
	TCHAR szWindowClass[100];
	HWND hWnd;
	HMENU hMenu;
	DWORD showedStyle;
	DWORD hidedStyle;
	bool showed;
	bool inDrawing;
	int topMargin;
	int leftMargin;
	std::vector<std::wstring> lines;
	RECT clientRect;
	TextFont* font;
	wchar_t* fontName;
	Subtitles subtitles; 
	std::wstring information;
	TextFont informationFont;
	boost::property_tree::ptree properties;
};

