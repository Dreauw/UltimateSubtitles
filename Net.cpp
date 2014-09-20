#include "stdafx.h"
#include "XmlRpc.h"
#include "Net.h"

HINTERNET Net::hInternet;


Net::Net(void) {}

void Net::initializeWininet() 
{
	if (hInternet == NULL) hInternet = InternetOpen(L"UltimateSubtitles", NULL, NULL, NULL, NULL);
}

HINTERNET Net::connect(const char* url)
{
	initializeWininet();
	return InternetConnectA(hInternet, url, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
}

void Net::disconnect(HINTERNET hConnect)
{
	InternetCloseHandle(hConnect);
}

void Net::sendXmlRpc(HINTERNET hConnect, const char* object, XmlRpc& args, XmlRpc& result)
{
	const char* acceptTypes[2] = { "text/*", NULL };
	int flags = INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION;
	std::string argsXml;
	args.toStr(argsXml);
	HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", object, NULL, NULL, acceptTypes, flags, NULL);
	// Callback
	char header[255];
	sprintf_s(header, "Content-Type: text/xml\r\nContent-length: %d", argsXml.size());
    BOOL a = HttpAddRequestHeadersA(hRequest, header, strlen(header), HTTP_ADDREQ_FLAG_ADD);
	BOOL s = HttpSendRequest(hRequest, NULL, 0, (LPVOID)argsXml.c_str(), argsXml.size());
	DWORD e = GetLastError();
	std::string buff;
	readFile(hRequest, buff);
	//print("hConnect : %u hRequest : %u add : %d  send : %u %s", hConnect, hRequest, a, e, buff.c_str());
	result.load(buff);
	InternetCloseHandle(hRequest);
}

int Net::download(std::string url, std::string* buffer)
{
	int size = 0;
	initializeWininet();
	HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, NULL, NULL, NULL);
	size = readFile(hFile, *buffer);
	InternetCloseHandle(hFile);
	return size;
}

int Net::download(std::string url, std::wstring* buffer)
{
	int size = 0;
	std::string tmpBuf;
	size = download(url, &tmpBuf);
	buffer->append(CA2W(tmpBuf.c_str(), CP_UTF8));
	return size;
}

int Net::readFile(HINTERNET hFile, std::string& buffer) 
{
	int size = 0;
	char tmp[1024];
	DWORD lengthReaded = 0;
	while (InternetReadFile(hFile, tmp, 1024, &lengthReaded) == TRUE && lengthReaded > 0)
	{
		buffer.append(tmp, lengthReaded);
		size += lengthReaded;
	}
	return size;
}

Net::~Net(void) {}
