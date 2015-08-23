#include "stdafx.h"
#include "resource.h"

#include "TextFont.h"
#include "Subtitle.h"
#include "Subtitles.h"
#include "Window.h"
#include "SearchBox.h"
#include "SubtitleBox.h"

SubtitleBox::SubtitleBox(Window& wnd, std::map<std::string, std::string>& subtitles) :
	window(wnd),
	subtitles(subtitles),
	result(NULL)
{
	DialogBoxParam(wnd.getInstance(), MAKEINTRESOURCE(IDD_SUBTITLEBOX), wnd.getWnd(), subtitleBoxProc, (LPARAM)this);
}

void SubtitleBox::onInit(HWND hDlg)
{
	HWND combo = GetDlgItem (hDlg, IDC_COMBO1);
	int idx = 0;
	for (std::map<std::string, std::string>::iterator it = subtitles.begin(); it != subtitles.end(); ++it)
	{
		std::wstring wstr(it->first.begin(), it->first.end());
		std::wstring wstr2(it->second.begin(), it->second.end());
		ComboBox_AddString(combo, wstr.c_str());
		ComboBox_SetItemData(combo, idx, (LPARAM) &it->second);
		++idx;
	}
	ComboBox_SetCurSel(combo, 1);
}


void SubtitleBox::onOk(HWND hDlg)
{
	HWND combo = GetDlgItem (hDlg, IDC_COMBO1);
	LRESULT sel = ComboBox_GetCurSel(combo);
	if (sel != CB_ERR) {
		std::string* res = (std::string*) ComboBox_GetItemData(combo, sel);
		if (res != NULL) {
			result = res;
		}
	}
}

std::string* SubtitleBox::getResult()
{
	return result;
}


SubtitleBox::~SubtitleBox(void)
{
}



INT_PTR CALLBACK SubtitleBox::subtitleBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG) SetWindowLong(hDlg, GWL_USERDATA, (LONG)lParam);

	SubtitleBox* subtitlebox = (SubtitleBox*)GetWindowLong(hDlg, GWL_USERDATA);
	if (subtitlebox != NULL) {
		switch (message) 
		{
		case WM_INITDIALOG:
			subtitlebox->onInit(hDlg);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				if (LOWORD(wParam) == IDOK) subtitlebox->onOk(hDlg);
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}