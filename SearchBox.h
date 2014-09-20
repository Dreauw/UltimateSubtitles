#pragma once

class SearchBox
{
public:
	SearchBox(Window& wnd);
	~SearchBox(void);
	static void show(HWND hWnd);
	static void createAutoCompletion(HWND edit);
	static INT_PTR CALLBACK SearchBox::proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static INT_PTR CALLBACK SearchBox::searchBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static void updateSuggestions(std::string& search);
	static void clearSuggestions() ;
	static void findSuggestions(char* text); 
	static void getBasicString(std::string& str);
	static std::string getImdbId(std::string& str);
	static void downloadSubtitle(std::string& imdbId, std::string& name, std::string& lang, int season, int episode);
	static void onOk(HWND dlg);

	typedef CComObject<CComEnum<IEnumString, &IID_IEnumString, wchar_t*, _Copy<wchar_t*> >> CComEnumString;
	static CComObject<CComEnumString>* enumStr;
	static CComPtr<IAutoComplete> autoComplete;
	static boost::property_tree::ptree suggestions;
	static std::vector<wchar_t*> titles;
	static std::vector<std::string> titlesId;
	static Window* window;
};