#include "stdafx.h"
#include "resource.h"

#include "TextFont.h"
#include "Subtitle.h"
#include "Subtitles.h"
#include "Window.h"
#include "SearchBox.h"
#include "KeyBox.h"

KeyBox::KeyBox(Window& wnd) :
	window(wnd)
{
	DialogBoxParam(wnd.getInstance(), MAKEINTRESOURCE(IDD_KEYBOX), wnd.getWnd(), keyBoxProc, (LPARAM)this);
}

void KeyBox::onInit(HWND hDlg)
{
	HWND ctrlHwnd;
	// Checkbox
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_PAUSE);
	Button_SetCheck(ctrlHwnd, window.getProperties().get("hotkeys.pause.enabled", false) ? BST_CHECKED : BST_UNCHECKED);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_PREVIOUS);
	Button_SetCheck(ctrlHwnd, window.getProperties().get("hotkeys.previous.enabled", false) ? BST_CHECKED : BST_UNCHECKED);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_NEXT);
	Button_SetCheck(ctrlHwnd, window.getProperties().get("hotkeys.next.enabled", false) ? BST_CHECKED : BST_UNCHECKED);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_DECREASE);
	Button_SetCheck(ctrlHwnd, window.getProperties().get("hotkeys.decrease.enabled", false) ? BST_CHECKED : BST_UNCHECKED);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_INCREASE);
	Button_SetCheck(ctrlHwnd, window.getProperties().get("hotkeys.increase.enabled", false) ? BST_CHECKED : BST_UNCHECKED);

	// Hotkeys
	SendDlgItemMessage(hDlg, IDC_HOTKEY_PAUSE, HKM_SETHOTKEY, window.getProperties().get("hotkeys.pause.keycode", 0), 0);
	SendDlgItemMessage(hDlg, IDC_HOTKEY_PREVIOUS, HKM_SETHOTKEY, window.getProperties().get("hotkeys.previous.keycode", 0), 0);
	SendDlgItemMessage(hDlg, IDC_HOTKEY_NEXT, HKM_SETHOTKEY, window.getProperties().get("hotkeys.next.keycode", 0), 0);
	SendDlgItemMessage(hDlg, IDC_HOTKEY_DECREASE, HKM_SETHOTKEY, window.getProperties().get("hotkeys.decrease.keycode", 0), 0);
	SendDlgItemMessage(hDlg, IDC_HOTKEY_INCREASE, HKM_SETHOTKEY, window.getProperties().get("hotkeys.increase.keycode", 0), 0);
}


void KeyBox::onOk(HWND hDlg)
{
	HWND ctrlHwnd;
	// Checkbox
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_PAUSE);
	window.getProperties().put("hotkeys.pause.enabled", Button_GetCheck(ctrlHwnd) == BST_CHECKED ? true : false);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_PREVIOUS);
	window.getProperties().put("hotkeys.previous.enabled", Button_GetCheck(ctrlHwnd) == BST_CHECKED ? true : false);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_NEXT);
	window.getProperties().put("hotkeys.next.enabled", Button_GetCheck(ctrlHwnd) == BST_CHECKED ? true : false);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_DECREASE);
	window.getProperties().put("hotkeys.decrease.enabled", Button_GetCheck(ctrlHwnd) == BST_CHECKED ? true : false);
	ctrlHwnd = GetDlgItem(hDlg, IDC_CHECK_INCREASE);
	window.getProperties().put("hotkeys.increase.enabled", Button_GetCheck(ctrlHwnd) == BST_CHECKED ? true : false);

	// Hotkeys
	int key;
	ctrlHwnd = GetDlgItem(hDlg, IDC_HOTKEY_PAUSE);
	key = SendMessage(ctrlHwnd, HKM_GETHOTKEY, 0, 0);
	if (HIBYTE(key) == 8) key = MAKEWORD(LOBYTE(key), 0);
	window.getProperties().put("hotkeys.pause.keycode", key);
	ctrlHwnd = GetDlgItem(hDlg, IDC_HOTKEY_PREVIOUS);
	key = SendMessage(ctrlHwnd, HKM_GETHOTKEY, 0, 0);
	if (HIBYTE(key) == 8) key = MAKEWORD(LOBYTE(key), 0);
	window.getProperties().put("hotkeys.previous.keycode", key);
	ctrlHwnd = GetDlgItem(hDlg, IDC_HOTKEY_NEXT);
	key = SendMessage(ctrlHwnd, HKM_GETHOTKEY, 0, 0);
	if (HIBYTE(key) == 8) key = MAKEWORD(LOBYTE(key), 0);
	window.getProperties().put("hotkeys.next.keycode", key);
	ctrlHwnd = GetDlgItem(hDlg, IDC_HOTKEY_DECREASE);
	key = SendMessage(ctrlHwnd, HKM_GETHOTKEY, 0, 0);
	if (HIBYTE(key) == 8) key = MAKEWORD(LOBYTE(key), 0);
	window.getProperties().put("hotkeys.decrease.keycode", key);
	ctrlHwnd = GetDlgItem(hDlg, IDC_HOTKEY_INCREASE);
	key = SendMessage(ctrlHwnd, HKM_GETHOTKEY, 0, 0);
	if (HIBYTE(key) == 8) key = MAKEWORD(LOBYTE(key), 0);
	window.getProperties().put("hotkeys.increase.keycode", key);

	window.updateHotkey();
}


KeyBox::~KeyBox(void)
{
}



INT_PTR CALLBACK KeyBox::keyBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG) SetWindowLong(hDlg, GWL_USERDATA, (LONG)lParam);

	KeyBox* keybox = (KeyBox*)GetWindowLong(hDlg, GWL_USERDATA);
	if (keybox != NULL) {
		switch (message) 
		{
		case WM_INITDIALOG:
			keybox->onInit(hDlg);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				if (LOWORD(wParam) == IDOK) keybox->onOk(hDlg);
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}