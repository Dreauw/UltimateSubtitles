#include "stdafx.h"
#include "resource.h"

#include "XmlRpc.h"
#include "Net.h"
#include "TextFont.h"
#include "Subtitle.h"
#include "Subtitles.h"
#include "Window.h"
#include "SearchBox.h"

class MyModule : public CAtlExeModuleT<MyModule> {};
MyModule _Module;

// Static variables
CComObject<SearchBox::CComEnumString>* SearchBox::enumStr;
boost::property_tree::ptree SearchBox::suggestions;
CComPtr<IAutoComplete> SearchBox::autoComplete;
std::vector<wchar_t*> SearchBox::titles;
std::vector<std::string> SearchBox::titlesId;
Window* SearchBox::window;

SearchBox::SearchBox(Window& wnd)
{
	window = &wnd;
}

void SearchBox::show(HWND hWnd) 
{
	DialogBox(NULL, MAKEINTRESOURCE(IDD_SEARCHBOX), hWnd, searchBoxProc);
}

void SearchBox::downloadSubtitle(std::string& imdbId, std::string& name, std::string& lang, int season, int episode)
{
	boost::property_tree::ptree resPt;
	XmlRpc res(resPt);
	std::string strApi = window->getProperties().get("opensubtitles.api", "");
	std::string  strUsername = window->getProperties().get("opensubtitles.username", "");
	std::string  strPassword = window->getProperties().get("opensubtitles.password", "");
	std::string  strUseragent = window->getProperties().get("opensubtitles.useragent", "");

	// LogIn (OpenSubtitle)
	HINTERNET hConnect = Net::connect((char*)strApi.c_str());
	boost::property_tree::ptree ptLogin;
	XmlRpc login(ptLogin, "LogIn");
	login[0] = (char*) strUsername.c_str();
	login[1] = (char*) strPassword.c_str();
	login[2] = "en";
	login[3] = (char*) strUseragent.c_str();
	Net::sendXmlRpc(hConnect, "/xml-rpc", login, res);
	std::string token = res[0]["token"].getString();

	// Search
	boost::property_tree::ptree ptSearch;
	XmlRpc search(ptSearch, "SearchSubtitles");
	search[0] = (char*)token.c_str();
	if (imdbId.empty()) {
		search[1][0]["query"] = (char*)name.c_str();
	} else {
		search[1][0]["imdbid"] = (char*)imdbId.c_str();
	}
	if (!lang.empty()) search[1][0]["sublanguageid"] = (char*)lang.c_str();
	if (season > 0) search[1][0]["season"] = season;
	if (episode > 0) search[1][0]["episode"] = episode;
	Net::sendXmlRpc(hConnect, "/xml-rpc", search, res);

	try {
		boost::property_tree::ptree& searchList = res[0]["data"].getTree().get_child("array.data");

		// Choose the best rated subtitle
		std::string downloadLink;
		std::string fileName;
		double rating = -1;
		int idx = 0;
		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, searchList)
		{
			// Should be in the top 8 of most downloaded
			if (idx > 8) break;

			XmlRpc entry(v.second);
			double tmpRating = ::atof(entry["SubRating"].getString().c_str());
			if (tmpRating > rating) {
			//if (idx == 1) {
				rating = tmpRating;
				downloadLink = entry["SubDownloadLink"].getString();
				fileName = entry["SubFileName"].getString();
			}
			idx++;
		}
		SetWindowTextA(window->getWnd(), ("UltimateSubtitles - " + fileName).c_str());

		// Download the subtitle
		std::string buffer;
	
		int size = Net::download(downloadLink, &buffer);
		// Decompress the subtitle
		char buff[1024];
		std::string subBuff;
		z_stream infstream;
		infstream.zalloc = Z_NULL;
		infstream.zfree = Z_NULL;
		infstream.opaque = Z_NULL;
		infstream.avail_in = (uInt)size; // size of input
		infstream.next_in = (Bytef *)buffer.c_str(); // input char array
		infstream.avail_out = 0;//(uInt)1024; // size of output
		infstream.next_out = (Bytef *)buff; // output char array
		inflateInit2(&infstream, 16+MAX_WBITS);
		while (infstream.avail_out == 0) {
			infstream.next_out = (Bytef *)buff;
			infstream.avail_out = (uInt)1023;
			inflate(&infstream, Z_NO_FLUSH);
			subBuff.append(buff, 1023-infstream.avail_out);
		}
		inflateEnd(&infstream);

		std::wstring utfBuffer;

		if (subBuff.substr(0, 3) == "ï»¿") {
			utfBuffer.append(CA2W(subBuff.c_str(), CP_UTF8));
		} else {
			CComPtr<IMultiLanguage2> spMLang;
 
			DetectEncodingInfo lpInfo[1];
			INT nScore = 1;
			INT nlen = subBuff.length();

			spMLang.CoCreateInstance(CLSID_CMultiLanguage);
			DWORD err = spMLang->DetectInputCodepage(MLDETECTCP_NONE, 0, (CHAR*)subBuff.c_str(), &nlen, lpInfo, &nScore);
			
			utfBuffer.append(CA2W(subBuff.c_str(), lpInfo[0].nCodePage));
		}
		boost::replace_all(utfBuffer, "\r\n", "\n");

		std::wistringstream str(utfBuffer);

		window->getSubtitles().load(str);
	} catch(...) {
		print("Unable to find this subtitle");
		return;
	}

	// LogOut
	boost::property_tree::ptree ptLogout;
	XmlRpc logout(ptLogout, "LogOut");
	logout[0] = (char*)token.c_str();
	Net::sendXmlRpc(hConnect, "/xml-rpc", logout, res);
	Net::disconnect(hConnect);
}


void SearchBox::createAutoCompletion(HWND edit) 
{
	enumStr = NULL;
	autoComplete = NULL;
	CComObject<CComEnumString>::CreateInstance(&enumStr);
	CComPtr<IEnumString> pEnumStr(enumStr);
	CComPtr<IAutoComplete2> autoComplete2;

	autoComplete.CoCreateInstance(CLSID_AutoComplete);
	autoComplete->Init(edit, pEnumStr, NULL, NULL);
	autoComplete->QueryInterface(IID_PPV_ARGS(&autoComplete2));
	autoComplete2->SetOptions(ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST | ACO_WORD_FILTER | ACO_NOPREFIXFILTERING);
}


// Make a string more "simple" by removing special characters and upper case
void SearchBox::getBasicString(std::string& str)
{
	boost::to_lower(str);
	std::replace(str.begin(), str.end(), ' ', '_');
	std::replace(str.begin(), str.end(), 'é', 'e');
	std::replace(str.begin(), str.end(), 'è', 'e');
	std::replace(str.begin(), str.end(), 'ê', 'e');
	std::replace(str.begin(), str.end(), 'à', 'a');
	std::replace(str.begin(), str.end(), 'ç', 'c');
	std::replace(str.begin(), str.end(), 'ï', 'i');
}


void SearchBox::findSuggestions(char* text) 
{
	std::wstring wdata;
	std::string search(text);
	if (search.length() <= 0) return;
	getBasicString(search);
	std::string url = "http://sg.media-imdb.com/suggests/";
	char firstChar[2] = {0};
	firstChar[0] = search[0];
	Net::download(url.append(firstChar).append("/").append(search).append(".json"), &wdata);
	std::string data(wdata.begin(), wdata.end());

	if (data.find("imdb$") != std::string::npos) {
		size_t startPos = data.find_first_of('(')+1;
		size_t endPos = data.find_last_of(')');
		using boost::property_tree::ptree;
		std::stringstream ss;
		ss << data.substr(startPos, endPos - startPos);
		read_json(ss, suggestions);
	}

	updateSuggestions(search);
}

void SearchBox::clearSuggestions() 
{
	if (titles.size() <= 0) return;

	for(std::vector<wchar_t*>::iterator it = titles.begin(); it != (titles.end() - 1); ++it) {
		delete *it;
	}
	titles.clear();
	titlesId.clear();
}


void SearchBox::updateSuggestions(std::string& search) 
{
	clearSuggestions();
	using boost::property_tree::ptree;
	try {
		BOOST_FOREACH(ptree::value_type &v, suggestions.get_child("d")) 
		{
			std::string id = v.second.get<std::string>("id");
			if (id.find("tt") != 0) continue;

			std::string title = v.second.get<std::string>("l");
			std::string searchTitle = title;
			getBasicString(searchTitle);
			std::vector<std::string> searchWords;
			boost::split(searchWords, search, boost::is_any_of("_"));
 
			bool contain = true;
			for (size_t i = 0; i < searchWords.size(); i++)
				if (searchTitle.find(searchWords[i]) == std::string::npos) contain = false;
		
			boost::optional<std::string> year = v.second.get_optional<std::string>("y");
			if (year) 
			{
					title.append(" (").append(year.get()).append(")");
					searchTitle.append("_(").append(year.get()).append(")");
			}

			if (search.find(searchTitle) != std::string::npos) titlesId.push_back(id);

			if (!contain) continue;

			size_t ret = 0;
			size_t size = title.length() + 1;
			wchar_t* wa = new wchar_t[size];
			mbstowcs_s(&ret, wa, size, title.c_str(), _TRUNCATE);
			titles.push_back(wa);
			titlesId.push_back(id);
		}
	} catch (...) {
		return;
	}

	if (titles.size() <= 0) return;
	titles.push_back(L"");

	((CComEnumString*)enumStr)->Init(&(titles[0]), &(titles[titles.size()-1]), NULL);
	CComPtr<IAutoCompleteDropDown> autoCompleteDropDown;
    autoComplete->QueryInterface(IID_PPV_ARGS(&autoCompleteDropDown));
    autoCompleteDropDown->ResetEnumerator(); 
}

// Return the imdbId corresponding to the str
std::string SearchBox::getImdbId(std::string& str)
{
	// Check if the movie is in the suggestions
	if (titlesId.size() > 0) {
		std::string str(titlesId[0]);
		str.erase(0, 2);
		return str;
	}
	
	return std::string("");
}

void SearchBox::onOk(HWND hDlg)
{
	char buff[256];
	GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT1), buff, 255);
	std::string name(buff);
	GetWindowTextA(GetDlgItem(hDlg, IDC_COMBO_LANG), buff, 255);
	std::string lang(buff);
	BOOL succeed;
	int episode = GetDlgItemInt(hDlg, IDC_EDIT_EPISODE, &succeed, TRUE);
	if (succeed == FALSE) episode = -1;
	int season = GetDlgItemInt(hDlg, IDC_EDIT_SEASON, &succeed, TRUE);
	if (succeed == FALSE) season = -1;
	std::string imdb = getImdbId(name);
	window->getProperties().put("search.movie", name);
	window->getProperties().put("search.imdb", imdb);
	window->getProperties().put("search.language", lang);
	window->getProperties().put("search.season", "");
	if (season > 0) window->getProperties().put("search.season", season);
	window->getProperties().put("search.episode", "");
	if (episode > 0) window->getProperties().put("search.episode", episode);
	downloadSubtitle(imdb, name, lang, season, episode);
}

INT_PTR CALLBACK SearchBox::searchBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
		{
			HWND combo = GetDlgItem (hDlg, IDC_COMBO_LANG);
			ComboBox_AddString(combo, L"eng");
			ComboBox_AddString(combo, L"fre");
			ComboBox_AddString(combo, L"spa");
			ComboBox_AddString(combo, L"dut");
			ComboBox_SetCurSel(combo, 1);
			std::string imdb = window->getProperties().get("search.imdb", "-1");
			if (imdb.compare("-1") != 0)  titlesId.push_back("tt"+imdb);
			SetWindowTextA(combo, window->getProperties().get("search.language", "").c_str());
			SetWindowTextA(GetDlgItem (hDlg, IDC_EDIT1), window->getProperties().get("search.movie", "").c_str());
			SetWindowTextA(GetDlgItem (hDlg, IDC_EDIT_EPISODE), window->getProperties().get("search.episode", "").c_str());
			SetWindowTextA(GetDlgItem (hDlg, IDC_EDIT_SEASON), window->getProperties().get("search.season", "").c_str());

			createAutoCompletion(GetDlgItem (hDlg, IDC_EDIT1));
			return (INT_PTR)TRUE;
		}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK) onOk(hDlg);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		// Update suggestions when the text change
		if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_EDIT1) 
		{
			char buff[256];
			GetWindowTextA((HWND)lParam, buff, 255);
			findSuggestions(buff);
		}
		break;
	case WM_NOTIFY:
		{
			UINT nCode = ((LPNMHDR)lParam)->code;
			if (nCode == UDN_DELTAPOS) {
				 NMUPDOWN nmupd = *((LPNMUPDOWN) lParam);
				 DWORD editId = (nmupd.hdr.idFrom == IDC_SPIN_SEASON ? IDC_EDIT_SEASON : IDC_EDIT_EPISODE);
				 int val = GetDlgItemInt(hDlg, editId, NULL, TRUE);
				 val -= nmupd.iDelta;
				 if (val >= 0) SetDlgItemInt(hDlg, editId, val, TRUE);
			}
			break;
		}
	case WM_DESTROY:
		autoComplete.Release();
		clearSuggestions();
		enumStr = NULL;
		autoComplete = NULL;
		break;
	}
	return (INT_PTR)FALSE;
}


SearchBox::~SearchBox(void) {}