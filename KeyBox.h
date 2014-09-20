#pragma once
class KeyBox
{
public:
	KeyBox(Window& wnd);
	~KeyBox(void);

private:
	static INT_PTR CALLBACK KeyBox::keyBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void onInit(HWND hDlg);
	void onOk(HWND hDlg);

	Window& window;
};

