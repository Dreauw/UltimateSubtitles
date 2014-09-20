#pragma once
class Net
{
public:
	Net(void);
	~Net(void);
	static int download(std::string url, std::wstring* buffer);
	static int download(std::string url, std::string* buffer);
	static void sendXmlRpc(HINTERNET hConnect, const char* object, XmlRpc& args, XmlRpc& result);
	static HINTERNET connect(const char* url);
	static void disconnect(HINTERNET hInternet);

private:
	static void initializeWininet();
	static int readFile(HINTERNET hFile, std::string& buffer);
	static HINTERNET hInternet;
};

