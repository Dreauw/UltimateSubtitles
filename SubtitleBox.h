#pragma once
class SubtitleBox
{
public:
	SubtitleBox(Window& wnd, std::map<std::string, std::string>& subtitles);
	~SubtitleBox(void);
	std::string* getResult();

private:
	static INT_PTR CALLBACK SubtitleBox::subtitleBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void onInit(HWND hDlg);
	void onOk(HWND hDlg);

	Window& window;
	std::map<std::string, std::string>& subtitles;
	std::string* result;
};

